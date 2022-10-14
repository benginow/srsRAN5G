/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "rlc_stress_test.h"
#include "srsgnb/pdcp/pdcp_factory.h"
#include "srsgnb/ran/bearer_logger.h"
#include "srsgnb/rlc/rlc_factory.h"
#include "srsgnb/support/executors/task_worker.h"
#include "srsgnb/support/srsgnb_assert.h"
#include <condition_variable>
#include <mutex>
#include <random>
#include <unistd.h>

using namespace srsgnb;

/// \brief Stack emulator used to stress test the RLC TX/RX entities.
/// This emulator will try to mimic a real gNB stack as closely as possible.
/// To do this, it will contain two executors (threads) the PCell executor and a UE executor.
/// The PCell executor will emulate the MAC TX by pulling PDUs from RLC;
/// while the UE executor will emulate the MAC RX and PDCP RX and TX.
class stress_stack
{
public:
  stress_stack(const stress_test_args& args, uint32_t id) :
    stack_id(id),
    args(args),
    ue_name("UE-Worker-" + std::to_string(id)),
    pcell_name("PCell-Worker-" + std::to_string(id)),
    ue_worker{ue_name, task_worker_queue_size, true},
    pcell_worker{pcell_name, task_worker_queue_size, true},
    logger("STACK", id, lcid_t{})
  {
    ue_executor    = make_task_executor(ue_worker);
    pcell_executor = make_task_executor(pcell_worker);

    // MAC
    mac = std::make_unique<mac_dummy>(args, id);

    // F1
    f1 = std::make_unique<f1_dummy>(id);

    // Trafic generators
    traffic_sink   = std::make_unique<stress_traffic_sink>(id);
    traffic_source = std::make_unique<stress_traffic_source>(args, id);

    // PDCP
    pdcp_config                  pdcp_cnfg = {};
    pdcp_entity_creation_message pdcp_msg  = {};
    pdcp_msg.config                        = pdcp_cnfg;
    pdcp_msg.tx_lower                      = f1.get();
    pdcp_msg.rx_upper_dn                   = traffic_sink.get();
    pdcp_msg.timers                        = &timers;
    pdcp                                   = create_pdcp_entity(pdcp_msg);
    traffic_source->set_pdcp_tx_upper(&pdcp->get_tx_upper_data_interface());
    f1->set_pdcp_rx_lower(&pdcp->get_rx_lower_interface());

    // RLC
    rlc_config                  rlc_cnfg = get_rlc_config_from_args(args);
    rlc_entity_creation_message rlc_msg  = {};
    rlc_msg.ue_index                     = static_cast<du_ue_index_t>(stack_id);
    rlc_msg.lcid                         = lcid_t{};
    rlc_msg.rx_upper_dn                  = f1.get();
    rlc_msg.tx_upper_cn                  = f1.get();
    rlc_msg.tx_upper_dn                  = f1.get();
    rlc_msg.tx_lower_dn                  = mac.get();
    rlc_msg.config                       = rlc_cnfg;
    rlc_msg.timers                       = &timers;
    rlc_msg.pcell_executor               = pcell_executor.get();
    rlc_msg.ue_executor                  = ue_executor.get();
    rlc                                  = create_rlc_entity(rlc_msg);
    f1->set_rlc_tx_upper_data(rlc->get_tx_upper_layer_data_interface());

    mac->set_rlc_tx_lower(rlc->get_tx_lower_layer_interface());
    mac->set_rlc_rx_lower(rlc->get_rx_lower_layer_interface());
  }

  ~stress_stack() = default;

  void start()
  {
    srsgnb_assert(peer_stack != nullptr, "Peer stack was not set when starting the stack.");
    srsgnb_assert(peer_stack != this, "Peer cannot be itself.");

    for (uint32_t i = 0; i < 20; i++) {
      traffic_source->send_pdu();
    }

    logger.log_debug("Starting stack workers");
    pcell_worker.start();
    ue_worker.start();

    // Schedule the TTI when the thread are started.
    pcell_executor->defer([this]() { run_lower_tti(0); });
  }

  void wait_for_finish()
  {
    logger.log_debug("Waiting for PCell worker to finish. Stack id={}\n", stack_id);
    {
      std::unique_lock<std::mutex> lk_pcell(mutex_pcell);
      cv_pcell.wait(lk_pcell, [this] { return stopping_pcell; });
    }
    logger.log_debug(
        "PCell worker no longer processing. Stack id={}, stopping={}\n", stack_id, stopping_pcell ? "true" : "false");

    logger.log_debug("Waiting to stop upper worker. Stack id={}\n", stack_id);
    {
      std::unique_lock<std::mutex> lk_ue(mutex_ue);
      cv_ue.wait(lk_ue, [this] { return stopping_ue; });
    }
    logger.log_debug("Upper thread no longer processing. Stack id={}", stack_id);

    pcell_worker.stop();
    ue_worker.stop();
  }

  void run_upper_tti(uint32_t tti)
  {
    logger.log_debug("Running upper TTI={}, PDU RX queue size={}", tti, mac->pdu_rx_list.size());
    if (tti < (args.nof_ttis - 1)) {
      for (uint32_t i = 0; i < 20; i++) {
        traffic_source->send_pdu();
      }
      mac->run_rx_tti();
      timers.tick_all(); // timers are run from the upper executor
    } else {
      std::unique_lock<std::mutex> lk(mutex_ue);
      stopping_ue = true;
      lk.unlock();
      cv_ue.notify_all();
    }
    pcell_executor->defer([this, tti]() { run_lower_tti(tti + 1); });
    logger.log_debug("Finished running upper TTI={}, PDU RX queue size={}", tti, mac->pdu_rx_list.size());
  }

  void run_lower_tti(uint32_t tti)
  {
    logger.log_debug("Running lower TTI={}", tti);
    if (tti < args.nof_ttis) {
      std::vector<byte_buffer_slice_chain> pdu_list = mac->run_tx_tti(tti);
      logger.log_debug("Generated PDU list size={}", pdu_list.size());
      ue_executor->defer([this, tti]() { run_upper_tti(tti); });
      peer_stack->push_pdus(std::move(pdu_list));
      tti++;
    } else {
      logger.log_debug("Stopping lower TTI={}", tti);
      std::unique_lock<std::mutex> lk(mutex_pcell);
      stopping_pcell = true;
      lk.unlock();
      cv_pcell.notify_all();
    }
    logger.log_debug("Finished running lower TTI={}", tti);
  }

  void push_pdus(std::vector<byte_buffer_slice_chain> list_pdus)
  {
    auto push_fnc = [this, list_pdus = std::move(list_pdus)]() mutable { mac->push_rx_pdus(std::move(list_pdus)); };
    if (!stopping_pcell && !stopping_ue) {
      ue_executor->defer(std::move(push_fnc));
    }
  }

  void set_peer_stack(stress_stack* peer_stack_) { peer_stack = peer_stack_; }

  pdcp_metrics_container       get_pdcp_metrics() { return pdcp->get_metrics(); }
  rlc_bearer_metrics_container get_rlc_metrics() { return rlc->get_metrics(); }

  // Mutex and condition variables for stopping workers
  std::mutex              mutex_pcell;
  std::mutex              mutex_ue;
  std::condition_variable cv_pcell;
  std::condition_variable cv_ue;
  bool                    stopping_pcell = false;
  bool                    stopping_ue    = false;

private:
  // Stack ID for logging
  uint16_t stack_id;

  // Arguments
  const stress_test_args& args;

  // Executors
  uint16_t                       task_worker_queue_size = 15000;
  std::string                    ue_name;
  std::string                    pcell_name;
  task_worker                    ue_worker;
  task_worker                    pcell_worker;
  std::unique_ptr<task_executor> ue_executor;
  std::unique_ptr<task_executor> pcell_executor;

  stress_stack* peer_stack = nullptr;

  // Timers
  timer_manager timers;

  // MAC
  std::unique_ptr<mac_dummy> mac = nullptr;

  // RLC
  std::unique_ptr<rlc_entity> rlc = nullptr;

  // F1
  std::unique_ptr<f1_dummy> f1 = nullptr;

  // PDCP
  std::unique_ptr<pdcp_entity> pdcp = nullptr;

  // Traffic generators
  std::unique_ptr<stress_traffic_sink>   traffic_sink   = nullptr;
  std::unique_ptr<stress_traffic_source> traffic_source = nullptr;

  // Logging
  srsgnb::bearer_logger logger;
};

void stress_test(const stress_test_args& args)
{
  auto& log_stack = srslog::fetch_basic_logger("STACK", false);

  // Create the stack emulators
  stress_stack stack_emulator_0(args, 0);
  stress_stack stack_emulator_1(args, 1);
  stack_emulator_0.set_peer_stack(&stack_emulator_1);
  stack_emulator_1.set_peer_stack(&stack_emulator_0);

  //  Launch transmission
  stack_emulator_0.start();
  stack_emulator_1.start();

  //  Wait for test to finish
  stack_emulator_0.wait_for_finish();
  stack_emulator_1.wait_for_finish();

  // Print and analyse metrics
  pdcp_metrics_container       stack0_pdcp_metrics = stack_emulator_0.get_pdcp_metrics();
  pdcp_metrics_container       stack1_pdcp_metrics = stack_emulator_1.get_pdcp_metrics();
  rlc_bearer_metrics_container stack0_rlc_metrics  = stack_emulator_0.get_rlc_metrics();
  rlc_bearer_metrics_container stack1_rlc_metrics  = stack_emulator_1.get_rlc_metrics();

  // Print PDCP metrics
  // log_stack.info("STACK 0 emulator PDCP TX metrics: {}", stack0_pdcp_metrics.tx);
  log_stack.info("STACK 0 emulator PDCP RX metrics: {}", stack0_pdcp_metrics.rx);
  // log_stack.info("STACK 1 emulator PDCP TX metrics: {}", stack1_pdcp_metrics.tx);
  log_stack.info("STACK 1 emulator PDCP RX metrics: {}", stack1_pdcp_metrics.rx);

  // Print RLC metrics
  log_stack.info("STACK 0 emulator RLC TX metrics: {}", stack0_rlc_metrics.tx);
  log_stack.info("STACK 0 emulator RLC RX metrics: {}", stack0_rlc_metrics.rx);
  log_stack.info("STACK 1 emulator RLC TX metrics: {}", stack1_rlc_metrics.tx);
  log_stack.info("STACK 1 emulator RLC RX metrics: {}", stack1_rlc_metrics.rx);
}

int main(int argc, char** argv)
{
  // srsgnb_debug_handle_crash(argc, argv);

  stress_test_args args = {};
  if (not parse_args(args, argc, argv)) {
    fprintf(stderr, "Could not parse command line options\n");
    return -1;
  }

  init_log_from_args(args);

  stress_test(args);

  return 0;
}
