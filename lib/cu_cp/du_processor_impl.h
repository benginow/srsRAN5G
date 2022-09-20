/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "adapters/f1ap_adapters.h"
#include "adapters/rrc_ue_adapters.h"
#include "srsgnb/adt/slot_array.h"
#include "srsgnb/asn1/rrc_nr/rrc_nr.h"
#include "srsgnb/cu_cp/cu_cp_types.h"
#include "srsgnb/cu_cp/du_processor_config.h"
#include "srsgnb/f1_interface/cu/f1c_cu.h"
#include "srsgnb/ran/nr_cgi.h"
#include "srsgnb/rrc/rrc_du_factory.h"
#include "srsgnb/support/async/async_task_loop.h"
#include "ue_manager.h"
#include <string>

namespace srsgnb {
namespace srs_cu_cp {

class du_processor_impl : public du_processor_interface
{
public:
  du_processor_impl(const du_processor_config_t du_processor_config_,
                    f1c_du_management_notifier& f1c_du_mgmt_notifier_,
                    f1c_message_notifier&       f1c_notifier_,
                    rrc_ue_nas_notifier&        rrc_ue_ngc_ev_notifier_);
  ~du_processor_impl() = default;

  // message handlers
  void handle_f1_setup_request(const f1_setup_request_message& msg) override;

  du_index_t get_du_index() override;

  // getter functions

  du_processor_context&   get_context() override { return context; };
  f1c_message_handler&    get_f1c_message_handler() override { return *f1ap; };
  f1c_statistics_handler& get_f1c_statistics_handler() override { return *f1ap; };

  size_t get_nof_ues() override { return ue_mng.get_nof_ues(); };

  // du_processor_rrc_message_handler
  ue_creation_complete_message handle_ue_creation_request(const ue_creation_message& msg) override;

  rrc_amf_connection_handler&
  get_rrc_amf_connection_handler() override; /// Pass handle to AMF connection handler within RRC

  // du_processor_rrc_ue_interface
  /// \brief Create SRB entry in bearer list and add adapter handle.
  void create_srb(const srb_creation_message& msg) override;

  void handle_ue_context_release_command(const ue_context_release_command_message& msg) override;

  void handle_ue_async_task(ue_index_t ue_index, async_task<void>&& task) override
  {
    ue_ctrl_loop[ue_index].schedule(std::move(task));
  }

  unique_timer   make_unique_timer() override { return timer_db.create_unique_timer(); }
  timer_manager& get_timer_manager() override { return timer_db; }

private:
  /// \brief Lookup the cell based on a given NR cell ID.
  /// \param[in] packed_nr_cell_id The packed NR cell ID received over F1AP.
  du_cell_index_t find_cell(uint64_t packed_nr_cell_id);

  /// \brief Get the next available DU cell index.
  /// \return The next DU cell index.
  du_cell_index_t get_next_du_cell_index();

  /// \brief Get the DU processor cell database.
  /// \return The DU processor cell database.
  slot_array<du_cell_context, MAX_NOF_DU_CELLS>& get_cell_db() { return cell_db; };

  // F1AP senders

  /// \brief Create and transmit the F1 Setup response message.
  /// \param[in] du_ctxt The context of the DU that should receive the message.
  void send_f1_setup_response(const du_processor_context& du_ctxt);

  /// \brief Create and transmit the F1 Setup failure message.
  /// \param[in] cause The cause of the failure.
  void send_f1_setup_failure(asn1::f1ap::cause_c::types::options cause);

  srslog::basic_logger& logger = srslog::fetch_basic_logger("CU-CP");
  du_processor_config_t cfg;

  f1c_du_management_notifier& f1c_du_mgmt_notifier;
  f1c_message_notifier&       f1c_notifier;
  rrc_ue_nas_notifier&        rrc_ue_ngc_ev_notifier;

  du_processor_context                          context;
  slot_array<du_cell_context, MAX_NOF_DU_CELLS> cell_db; /// flattened version of served cells list provided by DU/F1AP
  std::atomic<uint16_t>                         next_du_cell_index{0};

  // timers associated with a given DU.
  timer_manager timer_db;

  // Components
  std::unique_ptr<f1_interface>         f1ap;
  std::unique_ptr<rrc_du_ue_repository> rrc;

  ue_manager ue_mng;

  // F1AP to DU processor adapter
  f1ap_du_processor_adapter f1ap_ev_notifier;

  // RRC UE to DU processor adapter
  rrc_ue_du_processor_adapter rrc_ue_ev_notifier;

  // task event loops indexed by ue_index
  slot_array<async_task_sequencer, MAX_NOF_UES> ue_ctrl_loop;
};

} // namespace srs_cu_cp

} // namespace srsgnb
