/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "asn1_cell_group_config_helpers.h"
#include "../../asn1/asn1_diff_utils.h"
#include "srsgnb/support/error_handling.h"

using namespace srsgnb;
using namespace srsgnb::srs_du;
using namespace asn1::rrc_nr;

asn1::rrc_nr::coreset_s srsgnb::srs_du::make_asn1_rrc_coreset(const coreset_configuration& cfg)
{
  coreset_s cs;
  cs.coreset_id = cfg.id;
  cs.freq_domain_res.from_number(cfg.freq_domain_resources().to_uint64());
  cs.dur = cfg.duration;
  if (cfg.interleaved.has_value()) {
    auto& interv = cs.cce_reg_map_type.set_interleaved();
    asn1::number_to_enum(interv.reg_bundle_size, cfg.interleaved->reg_bundle_sz);
    asn1::number_to_enum(interv.interleaver_size, cfg.interleaved->interleaver_sz);
    interv.shift_idx_present = true;
    interv.shift_idx         = cfg.interleaved->shift_index;
  } else {
    cs.cce_reg_map_type.set_non_interleaved();
  }
  cs.precoder_granularity.value =
      cfg.precoder_granurality == coreset_configuration::precoder_granularity_type::same_as_reg_bundle
          ? coreset_s::precoder_granularity_opts::same_as_reg_bundle
          : coreset_s::precoder_granularity_opts::all_contiguous_rbs;
  cs.pdcch_dmrs_scrambling_id_present = cfg.pdcch_dmrs_scrambling_id.has_value();
  if (cs.pdcch_dmrs_scrambling_id_present) {
    cs.pdcch_dmrs_scrambling_id = *cfg.pdcch_dmrs_scrambling_id;
  }
  return cs;
}

asn1::rrc_nr::search_space_s srsgnb::srs_du::make_asn1_rrc_search_space(const search_space_configuration& cfg)
{
  search_space_s ss;
  ss.search_space_id                                = cfg.id;
  ss.coreset_id_present                             = true;
  ss.coreset_id                                     = cfg.cs_id;
  ss.monitoring_slot_periodicity_and_offset_present = true;
  search_space_s::monitoring_slot_periodicity_and_offset_c_::types period;
  bool success = asn1::number_to_enum(period, cfg.monitoring_slot_period);
  srsgnb_assert(success, "Invalid slot period");
  ss.monitoring_slot_periodicity_and_offset.set(period);
  switch (ss.monitoring_slot_periodicity_and_offset.type().value) {
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl1:
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl2:
      ss.monitoring_slot_periodicity_and_offset.sl2() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl4:
      ss.monitoring_slot_periodicity_and_offset.sl4() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl5:
      ss.monitoring_slot_periodicity_and_offset.sl5() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl8:
      ss.monitoring_slot_periodicity_and_offset.sl8() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl10:
      ss.monitoring_slot_periodicity_and_offset.sl10() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl16:
      ss.monitoring_slot_periodicity_and_offset.sl16() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl20:
      ss.monitoring_slot_periodicity_and_offset.sl20() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl40:
      ss.monitoring_slot_periodicity_and_offset.sl40() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl80:
      ss.monitoring_slot_periodicity_and_offset.sl80() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl160:
      ss.monitoring_slot_periodicity_and_offset.sl160() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl320:
      ss.monitoring_slot_periodicity_and_offset.sl320() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl640:
      ss.monitoring_slot_periodicity_and_offset.sl640() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl1280:
      ss.monitoring_slot_periodicity_and_offset.sl1280() = cfg.monitoring_slot_offset;
      break;
    case search_space_s::monitoring_slot_periodicity_and_offset_c_::types_opts::sl2560:
      ss.monitoring_slot_periodicity_and_offset.sl2560() = cfg.monitoring_slot_offset;
      break;
    default:
      srsgnb_assertion_failure("Invalid PDCCH slot offset={}", cfg.monitoring_slot_offset);
  }
  if (cfg.duration != 1) {
    ss.dur_present = true;
    ss.dur         = cfg.duration;
  }
  if (cfg.monitoring_symbols_within_slot.has_value()) {
    ss.monitoring_symbols_within_slot_present = true;
    ss.monitoring_symbols_within_slot.from_number(cfg.monitoring_symbols_within_slot->to_ulong());
  }
  ss.nrof_candidates_present = true;
  asn1::number_to_enum(ss.nrof_candidates.aggregation_level1, cfg.nof_candidates[0]);
  asn1::number_to_enum(ss.nrof_candidates.aggregation_level2, cfg.nof_candidates[1]);
  asn1::number_to_enum(ss.nrof_candidates.aggregation_level4, cfg.nof_candidates[2]);
  asn1::number_to_enum(ss.nrof_candidates.aggregation_level8, cfg.nof_candidates[3]);
  asn1::number_to_enum(ss.nrof_candidates.aggregation_level16, cfg.nof_candidates[4]);
  ss.search_space_type_present = true;
  if (cfg.type == search_space_configuration::type_t::common) {
    ss.search_space_type.set_common();
    ss.search_space_type.common().dci_format0_minus0_and_format1_minus0_present = cfg.common.f0_0_and_f1_0;
    ss.search_space_type.common().dci_format2_minus0_present                    = cfg.common.f2_0;
    ss.search_space_type.common().dci_format2_minus1_present                    = cfg.common.f2_1;
    ss.search_space_type.common().dci_format2_minus2_present                    = cfg.common.f2_2;
    ss.search_space_type.common().dci_format2_minus3_present                    = cfg.common.f2_3;
  } else {
    ss.search_space_type.set_ue_specific();
    ss.search_space_type.ue_specific().dci_formats.value =
        cfg.ue_specific == srsgnb::search_space_configuration::ue_specific_dci_format::f0_0_and_f1_0
            ? search_space_s::search_space_type_c_::ue_specific_s_::dci_formats_opts::formats0_minus0_and_minus1_minus0
            : search_space_s::search_space_type_c_::ue_specific_s_::dci_formats_opts::formats0_minus1_and_minus1_minus1;
  }
  return ss;
}

rlc_bearer_cfg_s make_asn1_rrc_rlc_bearer(const rlc_bearer_config& cfg)
{
  rlc_bearer_cfg_s out;

  out.lc_ch_id = cfg.lcid;

  out.served_radio_bearer_present = true;
  if (is_srb(cfg.lcid)) {
    out.served_radio_bearer.set_srb_id() = srb_id_to_uint(to_srb_id(cfg.lcid));
  } else {
    out.served_radio_bearer.set_drb_id() = drb_id_to_uint(cfg.drb_id);
  }

  out.rlc_cfg_present = true;
  switch (cfg.rlc_cfg.mode) {
    case rlc_mode::am: {
      rlc_cfg_c::am_s_& am              = out.rlc_cfg.set_am();
      am.ul_am_rlc.sn_field_len_present = true;
      asn1::number_to_enum(am.ul_am_rlc.sn_field_len, to_number(cfg.rlc_cfg.am.tx.sn_field_length));
      asn1::number_to_enum(am.ul_am_rlc.t_poll_retx, cfg.rlc_cfg.am.tx.t_poll_retx);
      asn1::number_to_enum(am.ul_am_rlc.poll_pdu, cfg.rlc_cfg.am.tx.poll_pdu);
      asn1::number_to_enum(am.ul_am_rlc.poll_byte, cfg.rlc_cfg.am.tx.poll_byte);
      asn1::number_to_enum(am.ul_am_rlc.max_retx_thres, cfg.rlc_cfg.am.tx.max_retx_thresh);

      am.dl_am_rlc.sn_field_len_present = true;
      asn1::number_to_enum(am.dl_am_rlc.sn_field_len, to_number(cfg.rlc_cfg.am.rx.sn_field_length));
      asn1::number_to_enum(am.dl_am_rlc.t_reassembly, cfg.rlc_cfg.am.rx.t_reassembly);
      asn1::number_to_enum(am.dl_am_rlc.t_status_prohibit, cfg.rlc_cfg.am.rx.t_status_prohibit);
    } break;
    case rlc_mode::um_bidir: {
      auto& um                          = out.rlc_cfg.set_um_bi_dir();
      um.ul_um_rlc.sn_field_len_present = true;
      um.ul_um_rlc.sn_field_len.value   = asn1::rrc_nr::sn_field_len_um_opts::size12;
      // TODO
    } break;
    case rlc_mode::um_unidir_dl: {
      auto& um                          = out.rlc_cfg.set_um_uni_dir_dl();
      um.dl_um_rlc.sn_field_len_present = true;
      um.dl_um_rlc.sn_field_len.value   = asn1::rrc_nr::sn_field_len_um_opts::size12;
      // TODO
    } break;
    case rlc_mode::um_unidir_ul: {
      auto& um                          = out.rlc_cfg.set_um_uni_dir_ul();
      um.ul_um_rlc.sn_field_len_present = true;
      um.ul_um_rlc.sn_field_len.value   = asn1::rrc_nr::sn_field_len_um_opts::size12;
      // TODO
    } break;
    default:
      report_fatal_error("Invalid RLC bearer configuration type");
  }

  out.mac_lc_ch_cfg_present                    = true;
  out.mac_lc_ch_cfg.ul_specific_params_present = true;
  out.mac_lc_ch_cfg.ul_specific_params.prio    = 1;
  out.mac_lc_ch_cfg.ul_specific_params.prioritised_bit_rate.value =
      lc_ch_cfg_s::ul_specific_params_s_::prioritised_bit_rate_opts::infinity;
  out.mac_lc_ch_cfg.ul_specific_params.bucket_size_dur.value =
      lc_ch_cfg_s::ul_specific_params_s_::bucket_size_dur_opts::ms5;
  out.mac_lc_ch_cfg.ul_specific_params.lc_ch_group_present          = true;
  out.mac_lc_ch_cfg.ul_specific_params.lc_ch_group                  = 0;
  out.mac_lc_ch_cfg.ul_specific_params.sched_request_id_present     = true;
  out.mac_lc_ch_cfg.ul_specific_params.sched_request_id             = 0;
  out.mac_lc_ch_cfg.ul_specific_params.lc_ch_sr_mask                = false;
  out.mac_lc_ch_cfg.ul_specific_params.lc_ch_sr_delay_timer_applied = false;

  return out;
}

void calculate_pdcch_config_diff(asn1::rrc_nr::pdcch_cfg_s& out, const pdcch_config& src, const pdcch_config& dest)
{
  calculate_addmodremlist_diff(
      out.coreset_to_add_mod_list,
      out.coreset_to_release_list,
      src.coresets,
      dest.coresets,
      [](const coreset_configuration& cs_cfg) { return make_asn1_rrc_coreset(cs_cfg); },
      [](const coreset_configuration& cs_cfg) { return (uint8_t)cs_cfg.id; });

  calculate_addmodremlist_diff(
      out.search_spaces_to_add_mod_list,
      out.search_spaces_to_release_list,
      src.search_spaces,
      dest.search_spaces,
      [](const search_space_configuration& ss) { return make_asn1_rrc_search_space(ss); },
      [](const search_space_configuration& ss) { return (uint8_t)ss.id; });

  // TODO: Remaining.
}

void make_asn1_rrc_dmrs_dl_for_pdsch(asn1::rrc_nr::dmrs_dl_cfg_s& out, const dmrs_downlink_config& cfg)
{
  if (cfg.type.has_value() && cfg.type.value() == dmrs_config_type::type2) {
    out.dmrs_type_present = true;
  }

  if (cfg.additional_positions.has_value()) {
    out.dmrs_add_position_present = true;
    switch (cfg.additional_positions.value()) {
      case dmrs_additional_positions::pos0:
        out.dmrs_add_position = dmrs_dl_cfg_s::dmrs_add_position_opts::pos0;
        break;
      case dmrs_additional_positions::pos1:
        out.dmrs_add_position = dmrs_dl_cfg_s::dmrs_add_position_opts::pos1;
        break;
      case dmrs_additional_positions::pos3:
        out.dmrs_add_position = dmrs_dl_cfg_s::dmrs_add_position_opts::pos3;
        break;
      default:
        srsgnb_assertion_failure("Invalid DMRS DL Add. Position={}", cfg.additional_positions.value());
    }
  }

  if (cfg.max_length.has_value() && cfg.max_length.value() != dmrs_max_length::len2) {
    // Note: Strange ASN1 generated code where there is no max length field.
    srsgnb_assertion_failure("Invalid DMRS DL max. length={}", cfg.max_length.value());
  }

  if (cfg.scrambling_id0.has_value()) {
    out.scrambling_id0_present = true;
    out.scrambling_id0         = cfg.scrambling_id0.value();
  }

  if (cfg.scrambling_id1.has_value()) {
    out.scrambling_id1_present = true;
    out.scrambling_id1         = cfg.scrambling_id1.value();
  }
}

void make_asn1_rrc_qcl_info(asn1::rrc_nr::qcl_info_s& out, const qcl_info& cfg)
{
  if (cfg.cell.has_value()) {
    out.cell_present = true;
    out.cell         = cfg.cell.value();
  }
  if (cfg.bwp_id.has_value()) {
    out.bwp_id_present = true;
    out.bwp_id         = cfg.bwp_id.value();
  }
  if (cfg.ref_sig.type == srsgnb::qcl_info::reference_signal::reference_signal_type::ssb) {
    auto& ssb = out.ref_sig.set_ssb();
    ssb       = cfg.ref_sig.ssb;
  } else if (cfg.ref_sig.type == srsgnb::qcl_info::reference_signal::reference_signal_type::csi_rs) {
    auto& csi_rs = out.ref_sig.set_csi_rs();
    csi_rs       = cfg.ref_sig.csi_rs;
  }
  switch (cfg.qcl_type) {
    case qcl_info::qcl_type::type_a:
      out.qcl_type = qcl_info_s::qcl_type_opts::type_a;
      break;
    case qcl_info::qcl_type::type_b:
      out.qcl_type = qcl_info_s::qcl_type_opts::type_b;
      break;
    case qcl_info::qcl_type::type_c:
      out.qcl_type = qcl_info_s::qcl_type_opts::type_c;
      break;
    case qcl_info::qcl_type::type_d:
      out.qcl_type = qcl_info_s::qcl_type_opts::type_d;
      break;
    default:
      srsgnb_assertion_failure("Invalid QCL Type={}", cfg.qcl_type);
  }
}

asn1::rrc_nr::pdsch_time_domain_res_alloc_s
make_asn1_rrc_pdsch_time_domain_alloc_list(const pdsch_time_domain_resource_allocation& cfg)
{
  pdsch_time_domain_res_alloc_s out{};
  out.k0_present = true;
  out.k0         = cfg.k0;
  switch (cfg.map_type) {
    case sch_mapping_type::typeA:
      out.map_type = pdsch_time_domain_res_alloc_s::map_type_opts::type_a;
      break;
    case sch_mapping_type::typeB:
      out.map_type = pdsch_time_domain_res_alloc_s::map_type_opts::type_b;
      break;
    default:
      srsgnb_assertion_failure("Invalid SCH mapping Type={}", cfg.map_type);
  }

  out.start_symbol_and_len = ofdm_symbol_range_to_sliv(cfg.symbols);

  return out;
}

asn1::rrc_nr::tci_state_s srsgnb::srs_du::make_asn1_rrc_tci_state(const tci_state& cfg)
{
  tci_state_s tci_st;
  tci_st.tci_state_id = cfg.state_id;
  make_asn1_rrc_qcl_info(tci_st.qcl_type1, cfg.qcl_type1);
  if (cfg.qcl_type2.has_value()) {
    tci_st.qcl_type2_present = true;
    make_asn1_rrc_qcl_info(tci_st.qcl_type2, cfg.qcl_type2.value());
  }

  return tci_st;
}

void calculate_pdsch_config_diff(asn1::rrc_nr::pdsch_cfg_s& out, const pdsch_config& src, const pdsch_config& dest)
{
  out.data_scrambling_id_pdsch_present = dest.data_scrambling_id_pdsch.has_value();
  if (out.data_scrambling_id_pdsch_present) {
    out.data_scrambling_id_pdsch = dest.data_scrambling_id_pdsch.value();
  }

  // DMRS Type A.
  if ((dest.pdsch_mapping_type_a_dmrs.has_value() && not src.pdsch_mapping_type_a_dmrs.has_value()) ||
      (dest.pdsch_mapping_type_a_dmrs.has_value() && src.pdsch_mapping_type_a_dmrs.has_value() &&
       dest.pdsch_mapping_type_a_dmrs != src.pdsch_mapping_type_a_dmrs)) {
    out.dmrs_dl_for_pdsch_map_type_a_present = true;
    make_asn1_rrc_dmrs_dl_for_pdsch(out.dmrs_dl_for_pdsch_map_type_a.set_setup(),
                                    dest.pdsch_mapping_type_a_dmrs.value());
  } else if (src.pdsch_mapping_type_a_dmrs.has_value() && not dest.pdsch_mapping_type_a_dmrs.has_value()) {
    out.dmrs_dl_for_pdsch_map_type_a_present = true;
    out.dmrs_dl_for_pdsch_map_type_a.set_release();
  }

  // DMRS Type B.
  if ((dest.pdsch_mapping_type_b_dmrs.has_value() && not src.pdsch_mapping_type_b_dmrs.has_value()) ||
      (dest.pdsch_mapping_type_b_dmrs.has_value() && src.pdsch_mapping_type_b_dmrs.has_value() &&
       dest.pdsch_mapping_type_b_dmrs != src.pdsch_mapping_type_b_dmrs)) {
    out.dmrs_dl_for_pdsch_map_type_b_present = true;
    make_asn1_rrc_dmrs_dl_for_pdsch(out.dmrs_dl_for_pdsch_map_type_b.set_setup(),
                                    dest.pdsch_mapping_type_b_dmrs.value());
  } else if (src.pdsch_mapping_type_b_dmrs.has_value() && not dest.pdsch_mapping_type_b_dmrs.has_value()) {
    out.dmrs_dl_for_pdsch_map_type_b_present = true;
    out.dmrs_dl_for_pdsch_map_type_b.set_release();
  }

  // TCI states.
  calculate_addmodremlist_diff(
      out.tci_states_to_add_mod_list,
      out.tci_states_to_release_list,
      src.tci_states,
      dest.tci_states,
      [](const tci_state& st) { return make_asn1_rrc_tci_state(st); },
      [](const tci_state& st) { return st.state_id; });

  // VRB-to-PRB Interleaver.
  if (dest.vrb_to_prb_itlvr.has_value()) {
    out.vrb_to_prb_interleaver_present = true;
    switch (dest.vrb_to_prb_itlvr.value()) {
      case pdsch_config::vrb_to_prb_interleaver::n2:
        out.vrb_to_prb_interleaver = pdsch_cfg_s::vrb_to_prb_interleaver_opts::n2;
        break;
      case pdsch_config::vrb_to_prb_interleaver::n4:
        out.vrb_to_prb_interleaver = pdsch_cfg_s::vrb_to_prb_interleaver_opts::n4;
        break;
      default:
        srsgnb_assertion_failure("Invalid VRB-to-PRB Interleaver={}", dest.vrb_to_prb_itlvr.value());
    }
  }

  // Resource Allocation type.
  switch (dest.res_alloc) {
    case pdsch_config::resource_allocation::resource_allocation_type_0:
      out.res_alloc = pdsch_cfg_s::res_alloc_opts::res_alloc_type0;
      break;
    case pdsch_config::resource_allocation::resource_allocation_type_1:
      out.res_alloc = pdsch_cfg_s::res_alloc_opts::res_alloc_type1;
      break;
    case pdsch_config::resource_allocation::dynamic_switch:
      out.res_alloc = pdsch_cfg_s::res_alloc_opts::dynamic_switch;
      break;
    default:
      srsgnb_assertion_failure("Invalid resource allocation type={}", dest.res_alloc);
  }

  // PDSCH Time Domain Allocation.
  if ((not dest.pdsch_td_alloc_list.empty() && src.pdsch_td_alloc_list.empty()) ||
      (not dest.pdsch_td_alloc_list.empty() && not src.pdsch_td_alloc_list.empty() &&
       dest.pdsch_td_alloc_list != src.pdsch_td_alloc_list)) {
    out.pdsch_time_domain_alloc_list_present = true;
    auto& alloc_list                         = out.pdsch_time_domain_alloc_list.set_setup();
    for (const auto& td_alloc : dest.pdsch_td_alloc_list) {
      alloc_list.push_back(make_asn1_rrc_pdsch_time_domain_alloc_list(td_alloc));
    }
  } else if (not src.pdsch_td_alloc_list.empty() && dest.pdsch_td_alloc_list.empty()) {
    out.pdsch_time_domain_alloc_list_present = true;
    out.pdsch_time_domain_alloc_list.set_release();
  }

  // RBG Size.
  switch (dest.rbg_sz) {
    case rbg_size::config1:
      out.rbg_size = pdsch_cfg_s::rbg_size_opts::cfg1;
      break;
    case rbg_size::config2:
      out.rbg_size = pdsch_cfg_s::rbg_size_opts::cfg2;
      break;
    default:
      srsgnb_assertion_failure("Invalid RBG size={}", dest.rbg_sz);
  }

  // PRB Bundling.
  if (variant_holds_alternative<prb_bundling::static_bundling>(dest.prb_bndlg.bundling)) {
    auto& st_bundling               = out.prb_bundling_type.set_static_bundling();
    st_bundling.bundle_size_present = true;
    const auto& bdlng               = variant_get<prb_bundling::static_bundling>(dest.prb_bndlg.bundling);
    switch (bdlng.sz.value()) {
      case prb_bundling::static_bundling::bundling_size::n4:
        st_bundling.bundle_size = pdsch_cfg_s::prb_bundling_type_c_::static_bundling_s_::bundle_size_opts::n4;
        break;
      case prb_bundling::static_bundling::bundling_size::wideband:
        st_bundling.bundle_size = pdsch_cfg_s::prb_bundling_type_c_::static_bundling_s_::bundle_size_opts::wideband;
        break;
      default:
        srsgnb_assertion_failure("Invalid static PRB bundling size={}", bdlng.sz.value());
    }
  } else {
    // Dynamic bundling.
    auto& dy_bundling                    = out.prb_bundling_type.set_dynamic_bundling();
    dy_bundling.bundle_size_set1_present = true;
    const auto& bdlng                    = variant_get<prb_bundling::dynamic_bundling>(dest.prb_bndlg.bundling);
    switch (bdlng.sz_set1.value()) {
      case prb_bundling::dynamic_bundling::bundling_size_set1::n4:
        dy_bundling.bundle_size_set1 =
            pdsch_cfg_s::prb_bundling_type_c_::dynamic_bundling_s_::bundle_size_set1_opts::n4;
        break;
      case prb_bundling::dynamic_bundling::bundling_size_set1::wideband:
        dy_bundling.bundle_size_set1 =
            pdsch_cfg_s::prb_bundling_type_c_::dynamic_bundling_s_::bundle_size_set1_opts::wideband;
        break;
      case prb_bundling::dynamic_bundling::bundling_size_set1::n2_wideband:
        dy_bundling.bundle_size_set1 =
            pdsch_cfg_s::prb_bundling_type_c_::dynamic_bundling_s_::bundle_size_set1_opts::n2_wideband;
        break;
      case prb_bundling::dynamic_bundling::bundling_size_set1::n4_wideband:
        dy_bundling.bundle_size_set1 =
            pdsch_cfg_s::prb_bundling_type_c_::dynamic_bundling_s_::bundle_size_set1_opts::n4_wideband;
        break;
      default:
        srsgnb_assertion_failure("Invalid dynamic PRB bundling set 1 size={}", bdlng.sz_set1.value());
    }
    dy_bundling.bundle_size_set2_present = true;
    switch (bdlng.sz_set2.value()) {
      case prb_bundling::dynamic_bundling::bundling_size_set2::n4:
        dy_bundling.bundle_size_set2 =
            pdsch_cfg_s::prb_bundling_type_c_::dynamic_bundling_s_::bundle_size_set2_opts::n4;
        break;
      case prb_bundling::dynamic_bundling::bundling_size_set2::wideband:
        dy_bundling.bundle_size_set2 =
            pdsch_cfg_s::prb_bundling_type_c_::dynamic_bundling_s_::bundle_size_set2_opts::wideband;
        break;
      default:
        srsgnb_assertion_failure("Invalid dynamic PRB bundling set 2 size={}", bdlng.sz_set2.value());
    }
  }

  // TODO: Remaining.
}

void calculate_bwp_dl_dedicated_diff(asn1::rrc_nr::bwp_dl_ded_s&   out,
                                     const bwp_downlink_dedicated& src,
                                     const bwp_downlink_dedicated& dest)
{
  if ((dest.pdcch_cfg.has_value() && not src.pdcch_cfg.has_value()) ||
      (dest.pdcch_cfg.has_value() && src.pdcch_cfg.has_value() && dest.pdcch_cfg != src.pdcch_cfg)) {
    out.pdcch_cfg_present = true;
    calculate_pdcch_config_diff(out.pdcch_cfg.set_setup(),
                                src.pdcch_cfg.has_value() ? src.pdcch_cfg.value() : pdcch_config{},
                                dest.pdcch_cfg.value());
  } else if (src.pdcch_cfg.has_value() && not dest.pdcch_cfg.has_value()) {
    out.pdcch_cfg_present = true;
    out.pdcch_cfg.set_release();
  }

  if ((dest.pdsch_cfg.has_value() && not src.pdsch_cfg.has_value()) ||
      (dest.pdsch_cfg.has_value() && src.pdsch_cfg.has_value() && dest.pdsch_cfg != src.pdsch_cfg)) {
    out.pdsch_cfg_present = true;
    calculate_pdsch_config_diff(out.pdsch_cfg.set_setup(),
                                src.pdsch_cfg.has_value() ? src.pdsch_cfg.value() : pdsch_config{},
                                dest.pdsch_cfg.value());
  } else if (src.pdsch_cfg.has_value() && not dest.pdsch_cfg.has_value()) {
    out.pdsch_cfg_present = true;
    out.pdsch_cfg.set_release();
  }
  // TODO: sps-Config and radioLinkMonitoringConfig.
}

asn1::rrc_nr::pucch_res_set_s srsgnb::srs_du::make_asn1_rrc_pucch_resource_set(const pucch_resource_set& cfg)
{
  pucch_res_set_s pucch_res_set;
  pucch_res_set.pucch_res_set_id = cfg.pucch_res_set_id;
  for (const auto& it : cfg.pucch_res_id_list) {
    pucch_res_set.res_list.push_back(it);
  }
  if (cfg.max_payload_size.has_value()) {
    pucch_res_set.max_payload_size = cfg.max_payload_size.value();
  }
  return pucch_res_set;
}

void make_asn1_rrc_pucch_formats_common_param(asn1::rrc_nr::pucch_format_cfg_s& out,
                                              const pucch_common_all_formats&   cfg)
{
  out.interslot_freq_hop_present = cfg.interslot_freq_hop;
  out.add_dmrs_present           = cfg.additional_dmrs;
  out.pi2_bpsk_present           = cfg.pi_2_bpsk;
  out.simul_harq_ack_csi_present = cfg.simultaneous_harq_ack_csi;
  if (cfg.max_c_rate == pucch_common_all_formats::max_code_rate::not_set) {
    out.max_code_rate_present = false;
  } else {
    out.max_code_rate_present = true;
    switch (cfg.max_c_rate) {
      case pucch_common_all_formats::max_code_rate::dot_08:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot08;
        break;
      case pucch_common_all_formats::max_code_rate::dot_15:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot15;
        break;
      case pucch_common_all_formats::max_code_rate::dot_25:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot25;
        break;
      case pucch_common_all_formats::max_code_rate::dot_35:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot35;
        break;
      case pucch_common_all_formats::max_code_rate::dot_45:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot45;
        break;
      case pucch_common_all_formats::max_code_rate::dot_60:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot60;
        break;
      case pucch_common_all_formats::max_code_rate::dot_80:
        out.max_code_rate = pucch_max_code_rate_opts::zero_dot80;
        break;
      default:
        srsgnb_assertion_failure("Invalid PUCCH Common Format 1 Max. Code Rate={}", cfg.max_c_rate);
    }
  }
  if (cfg.nof_slots == pucch_common_all_formats::num_of_slots::not_set) {
    out.nrof_slots_present = false;
  } else {
    out.nrof_slots_present = true;
    switch (cfg.nof_slots) {
      case pucch_common_all_formats::num_of_slots::n2:
        out.nrof_slots = pucch_format_cfg_s::nrof_slots_opts::n2;
        break;
      case pucch_common_all_formats::num_of_slots::n4:
        out.nrof_slots = pucch_format_cfg_s::nrof_slots_opts::n4;
        break;
      case pucch_common_all_formats::num_of_slots::n8:
        out.nrof_slots = pucch_format_cfg_s::nrof_slots_opts::n8;
        break;
      default:
        srsgnb_assertion_failure("Invalid PUCCH Common Format 1 No. of slots={}", cfg.nof_slots);
    }
  }
}

asn1::rrc_nr::pucch_res_s srsgnb::srs_du::make_asn1_rrc_pucch_resource(const pucch_resource& cfg)
{
  pucch_res_s pucch_res;
  pucch_res.pucch_res_id                = cfg.res_id;
  pucch_res.start_prb                   = cfg.starting_prb;
  pucch_res.intra_slot_freq_hop_present = cfg.intraslot_freq_hopping;
  pucch_res.second_hop_prb              = cfg.second_hop_prb;
  switch (cfg.format) {
    case pucch_format::FORMAT_0: {
      auto& format0             = pucch_res.format.set_format0();
      format0.init_cyclic_shift = cfg.format_0.initial_cyclic_shift;
      format0.nrof_symbols      = cfg.format_0.nof_symbols;
      format0.start_symbol_idx  = cfg.format_0.starting_sym_idx;
    } break;
    case pucch_format::FORMAT_1: {
      auto& format1             = pucch_res.format.set_format1();
      format1.init_cyclic_shift = cfg.format_1.initial_cyclic_shift;
      format1.nrof_symbols      = cfg.format_1.nof_symbols;
      format1.start_symbol_idx  = cfg.format_1.starting_sym_idx;
      format1.time_domain_occ   = cfg.format_1.time_domain_occ;
    } break;
    case pucch_format::FORMAT_2: {
      auto& format2            = pucch_res.format.set_format2();
      format2.start_symbol_idx = cfg.format_2.starting_sym_idx;
      format2.nrof_symbols     = cfg.format_2.nof_symbols;
      format2.nrof_prbs        = cfg.format_2.nof_prbs;
    } break;
    case pucch_format::FORMAT_3: {
      auto& format3            = pucch_res.format.set_format3();
      format3.start_symbol_idx = cfg.format_3.starting_sym_idx;
      format3.nrof_symbols     = cfg.format_3.nof_symbols;
      format3.nrof_prbs        = cfg.format_3.nof_prbs;
    } break;
    case pucch_format::FORMAT_4: {
      auto& format4            = pucch_res.format.set_format4();
      format4.start_symbol_idx = cfg.format_4.starting_sym_idx;
      format4.nrof_symbols     = cfg.format_4.nof_symbols;
      switch (cfg.format_4.occ_index) {
        case pucch_f4_occ_idx::n0:
          format4.occ_idx = pucch_format4_s::occ_idx_opts::n0;
          break;
        case pucch_f4_occ_idx::n1:
          format4.occ_idx = pucch_format4_s::occ_idx_opts::n1;
          break;
        case pucch_f4_occ_idx::n2:
          format4.occ_idx = pucch_format4_s::occ_idx_opts::n2;
          break;
        case pucch_f4_occ_idx::n3:
          format4.occ_idx = pucch_format4_s::occ_idx_opts::n3;
          break;
        default:
          srsgnb_assertion_failure("Invalid PUCCH Format 4 index={}", cfg.format_4.occ_index);
      }
      switch (cfg.format_4.occ_length) {
        case pucch_f4_occ_len::n2:
          format4.occ_len = pucch_format4_s::occ_len_opts::n2;
          break;
        case pucch_f4_occ_len::n4:
          format4.occ_len = pucch_format4_s::occ_len_opts::n4;
          break;
        default:
          srsgnb_assertion_failure("Invalid PUCCH Format 4 length={}", cfg.format_4.occ_length);
      }
    } break;
    default:
      srsgnb_assertion_failure("Invalid PDCCH resource format={}", cfg.format);
  }
  return pucch_res;
}

asn1::rrc_nr::sched_request_res_cfg_s
srsgnb::srs_du::make_asn1_rrc_sr_resource(const scheduling_request_resource_config& cfg)
{
  sched_request_res_cfg_s sr_res_cfg;
  sr_res_cfg.sched_request_res_id           = cfg.sr_res_id;
  sr_res_cfg.sched_request_id               = cfg.sr_id;
  sr_res_cfg.res_present                    = true;
  sr_res_cfg.res                            = cfg.pucch_res_id;
  sr_res_cfg.periodicity_and_offset_present = true;
  switch (cfg.period) {
    case sr_periodicity::sym_2:
      sr_res_cfg.periodicity_and_offset.set_sym2();
      break;
    case sr_periodicity::sym_6_or_7:
      sr_res_cfg.periodicity_and_offset.set_sym6or7();
      break;
    case sr_periodicity::sl_1:
      sr_res_cfg.periodicity_and_offset.set_sl1();
      break;
    case sr_periodicity::sl_2: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl2();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_4: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl4();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_5: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl5();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_8: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl8();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_10: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl10();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_16: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl16();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_20: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl20();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_40: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl40();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_80: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl80();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_160: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl160();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_320: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl320();
      period_and_offset       = cfg.offset;
    } break;
    case sr_periodicity::sl_640: {
      auto& period_and_offset = sr_res_cfg.periodicity_and_offset.set_sl640();
      period_and_offset       = cfg.offset;
    } break;
    default:
      srsgnb_assertion_failure("Invalid SR periodicity={}", cfg.period);
  }
  return sr_res_cfg;
}

void calculate_pucch_config_diff(asn1::rrc_nr::pucch_cfg_s& out, const pucch_config& src, const pucch_config& dest)
{
  // PUCCH Resource Set.
  calculate_addmodremlist_diff(
      out.res_set_to_add_mod_list,
      out.res_set_to_release_list,
      src.pucch_res_set,
      dest.pucch_res_set,
      [](const pucch_resource_set& res_set) { return make_asn1_rrc_pucch_resource_set(res_set); },
      [](const pucch_resource_set& res_set) { return res_set.pucch_res_set_id; });

  // PUCCH Resource.
  calculate_addmodremlist_diff(
      out.res_to_add_mod_list,
      out.res_to_release_list,
      src.pucch_res_list,
      dest.pucch_res_list,
      [](const pucch_resource& res) { return make_asn1_rrc_pucch_resource(res); },
      [](const pucch_resource& res) { return res.res_id; });

  if ((dest.format_1_common_param.has_value() && not src.format_1_common_param.has_value()) ||
      (dest.format_1_common_param.has_value() && src.format_1_common_param.has_value() &&
       dest.format_1_common_param != src.format_1_common_param)) {
    out.format1_present = true;
    make_asn1_rrc_pucch_formats_common_param(out.format1.set_setup(), dest.format_1_common_param.value());
  } else if (src.format_1_common_param.has_value() && not dest.format_1_common_param.has_value()) {
    out.format1_present = true;
    out.format1.set_release();
  }
  if ((dest.format_2_common_param.has_value() && not src.format_2_common_param.has_value()) ||
      (dest.format_2_common_param.has_value() && src.format_2_common_param.has_value() &&
       dest.format_2_common_param != src.format_2_common_param)) {
    out.format2_present = true;
    make_asn1_rrc_pucch_formats_common_param(out.format2.set_setup(), dest.format_2_common_param.value());
  } else if (src.format_2_common_param.has_value() && not dest.format_2_common_param.has_value()) {
    out.format2_present = true;
    out.format2.set_release();
  }
  if ((dest.format_3_common_param.has_value() && not src.format_3_common_param.has_value()) ||
      (dest.format_3_common_param.has_value() && src.format_3_common_param.has_value() &&
       dest.format_3_common_param != src.format_3_common_param)) {
    out.format3_present = true;
    make_asn1_rrc_pucch_formats_common_param(out.format3.set_setup(), dest.format_3_common_param.value());
  } else if (src.format_3_common_param.has_value() && not dest.format_3_common_param.has_value()) {
    out.format3_present = true;
    out.format3.set_release();
  }
  if ((dest.format_4_common_param.has_value() && not src.format_4_common_param.has_value()) ||
      (dest.format_4_common_param.has_value() && src.format_4_common_param.has_value() &&
       dest.format_4_common_param != src.format_4_common_param)) {
    out.format4_present = true;
    make_asn1_rrc_pucch_formats_common_param(out.format4.set_setup(), dest.format_4_common_param.value());
  } else if (src.format_4_common_param.has_value() && not dest.format_4_common_param.has_value()) {
    out.format4_present = true;
    out.format4.set_release();
  }

  // SR Resource.
  calculate_addmodremlist_diff(
      out.sched_request_res_to_add_mod_list,
      out.sched_request_res_to_release_list,
      src.sr_res_list,
      dest.sr_res_list,
      [](const scheduling_request_resource_config& res) { return make_asn1_rrc_sr_resource(res); },
      [](const scheduling_request_resource_config& res) { return res.sr_id; });

  // dl-DataToUL-ACK.
  for (const auto& timing : dest.dl_data_to_ul_ack) {
    out.dl_data_to_ul_ack.push_back(timing);
  }
}

void make_asn1_rrc_ptrs_ul_cfg(asn1::rrc_nr::ptrs_ul_cfg_s& out, const ptrs_uplink_config& cfg)
{
  if (cfg.trans_precoder_disabled.has_value()) {
    out.transform_precoder_disabled_present = true;

    const auto& cfg_trans_preco_dis = cfg.trans_precoder_disabled.value();
    auto&       out_trans_preco_dis = out.transform_precoder_disabled;

    for (unsigned idx = 0; idx < cfg_trans_preco_dis.t_density.size(); idx++) {
      out_trans_preco_dis.time_density[idx] = cfg_trans_preco_dis.t_density[idx];
    }

    for (unsigned idx = 0; idx < cfg_trans_preco_dis.f_density.size(); idx++) {
      out_trans_preco_dis.freq_density[idx] = cfg_trans_preco_dis.f_density[idx];
    }

    switch (cfg_trans_preco_dis.max_ports) {
      case ptrs_uplink_config::transform_precoder_disabled::max_nof_ports::n1:
        out_trans_preco_dis.max_nrof_ports = ptrs_ul_cfg_s::transform_precoder_disabled_s_::max_nrof_ports_opts::n1;
        break;
      case ptrs_uplink_config::transform_precoder_disabled::max_nof_ports::n2:
        out_trans_preco_dis.max_nrof_ports = ptrs_ul_cfg_s::transform_precoder_disabled_s_::max_nrof_ports_opts::n2;
        break;
      default:
        srsgnb_assertion_failure("Invalid PTRS UL Cfg Max. Ports={}", cfg_trans_preco_dis.max_ports);
    }

    if (cfg_trans_preco_dis.res_elem_offset !=
        srsgnb::ptrs_uplink_config::transform_precoder_disabled::resource_element_offset::not_set) {
      switch (cfg_trans_preco_dis.res_elem_offset) {
        case ptrs_uplink_config::transform_precoder_disabled::resource_element_offset::offset01:
          out_trans_preco_dis.res_elem_offset =
              ptrs_ul_cfg_s::transform_precoder_disabled_s_::res_elem_offset_opts::offset01;
          break;
        case ptrs_uplink_config::transform_precoder_disabled::resource_element_offset::offset10:
          out_trans_preco_dis.res_elem_offset =
              ptrs_ul_cfg_s::transform_precoder_disabled_s_::res_elem_offset_opts::offset10;
          break;
        case ptrs_uplink_config::transform_precoder_disabled::resource_element_offset::offset11:
          out_trans_preco_dis.res_elem_offset =
              ptrs_ul_cfg_s::transform_precoder_disabled_s_::res_elem_offset_opts::offset11;
          break;
        default:
          srsgnb_assertion_failure("Invalid Resource Element Offset={}", cfg_trans_preco_dis.res_elem_offset);
      }
    }

    switch (cfg_trans_preco_dis.power) {
      case ptrs_uplink_config::transform_precoder_disabled::ptrs_power::p00:
        out_trans_preco_dis.ptrs_pwr = ptrs_ul_cfg_s::transform_precoder_disabled_s_::ptrs_pwr_opts::p00;
        break;
      case ptrs_uplink_config::transform_precoder_disabled::ptrs_power::p01:
        out_trans_preco_dis.ptrs_pwr = ptrs_ul_cfg_s::transform_precoder_disabled_s_::ptrs_pwr_opts::p01;
        break;
      case ptrs_uplink_config::transform_precoder_disabled::ptrs_power::p10:
        out_trans_preco_dis.ptrs_pwr = ptrs_ul_cfg_s::transform_precoder_disabled_s_::ptrs_pwr_opts::p10;
        break;
      case ptrs_uplink_config::transform_precoder_disabled::ptrs_power::p11:
        out_trans_preco_dis.ptrs_pwr = ptrs_ul_cfg_s::transform_precoder_disabled_s_::ptrs_pwr_opts::p11;
        break;
      default:
        srsgnb_assertion_failure("Invalid PTRS UL Cfg Power={}", cfg_trans_preco_dis.power);
    }
  }

  if (cfg.trans_precoder_enabled.has_value()) {
    out.transform_precoder_enabled_present = true;

    const auto& cfg_trans_preco_enbl = cfg.trans_precoder_enabled.value();
    auto&       out_trans_preco_enbl = out.transform_precoder_enabled;

    for (unsigned idx = 0; idx < cfg_trans_preco_enbl.sampl_density.size(); idx++) {
      out_trans_preco_enbl.sample_density[idx] = cfg_trans_preco_enbl.sampl_density[idx];
    }
    if (cfg_trans_preco_enbl.is_t_density_trans_precoding_d2) {
      out_trans_preco_enbl.time_density_transform_precoding_present = true;
    }
  }
}

void make_asn1_rrc_dmrs_ul_for_pusch(asn1::rrc_nr::dmrs_ul_cfg_s& out,
                                     const dmrs_uplink_config&    src,
                                     const dmrs_uplink_config&    dest)
{
  if (dest.is_dmrs_type2) {
    out.dmrs_type_present = true;
  }

  if (dest.additional_positions != srsgnb::dmrs_additional_positions::not_set) {
    out.dmrs_add_position_present = true;
    switch (dest.additional_positions) {
      case dmrs_additional_positions::pos0:
        out.dmrs_add_position = dmrs_ul_cfg_s::dmrs_add_position_opts::pos0;
        break;
      case dmrs_additional_positions::pos1:
        out.dmrs_add_position = dmrs_ul_cfg_s::dmrs_add_position_opts::pos1;
        break;
      case dmrs_additional_positions::pos3:
        out.dmrs_add_position = dmrs_ul_cfg_s::dmrs_add_position_opts::pos3;
        break;
      default:
        srsgnb_assertion_failure("Invalid UL DMRS Add. pos={}", dest.additional_positions);
    }
  }

  if ((dest.ptrs.has_value() && not src.ptrs.has_value()) ||
      (dest.ptrs.has_value() && src.ptrs.has_value() && dest.ptrs != src.ptrs)) {
    out.phase_tracking_rs_present = true;
    make_asn1_rrc_ptrs_ul_cfg(out.phase_tracking_rs.set_setup(), dest.ptrs.value());
  } else if (src.ptrs.has_value() && not dest.ptrs.has_value()) {
    out.phase_tracking_rs_present = true;
    out.phase_tracking_rs.set_release();
  }

  if (dest.is_max_length_len2) {
    out.max_len_present = true;
  }

  if (dest.trans_precoder_disabled.has_value()) {
    out.transform_precoding_disabled_present = true;

    if (dest.trans_precoder_disabled.value().scrambling_id0.has_value()) {
      out.transform_precoding_disabled.scrambling_id0_present = true;
      out.transform_precoding_disabled.scrambling_id0 = dest.trans_precoder_disabled.value().scrambling_id0.value();
    }

    if (dest.trans_precoder_disabled.value().scrambling_id1.has_value()) {
      out.transform_precoding_disabled.scrambling_id1_present = true;
      out.transform_precoding_disabled.scrambling_id1 = dest.trans_precoder_disabled.value().scrambling_id1.value();
    }
  }

  if (dest.trans_precoder_enabled.has_value()) {
    out.transform_precoding_enabled_present = true;

    if (dest.trans_precoder_enabled.value().n_pusch_id.has_value()) {
      out.transform_precoding_enabled.npusch_id_present = true;
      out.transform_precoding_enabled.npusch_id         = dest.trans_precoder_enabled.value().n_pusch_id.value();
    }

    if (dest.trans_precoder_enabled.value().is_seq_grp_hopping_disabled) {
      out.transform_precoding_enabled.seq_group_hop_present = true;
    }

    if (dest.trans_precoder_enabled.value().is_seq_hopping_enabled) {
      out.transform_precoding_enabled.seq_hop_present = true;
    }
  }
}

asn1::rrc_nr::pusch_pathloss_ref_rs_s
srsgnb::srs_du::make_asn1_rrc_pusch_pathloss_ref_rs(const pusch_config::pusch_power_control::pusch_pathloss_ref_rs& cfg)
{
  pusch_pathloss_ref_rs_s ploss_ref_rs;
  ploss_ref_rs.pusch_pathloss_ref_rs_id = cfg.id;
  if (variant_holds_alternative<nzp_csi_rs_res_id_t>(cfg.rs)) {
    auto& csi_rs_idx = ploss_ref_rs.ref_sig.set_csi_rs_idx();
    csi_rs_idx       = variant_get<nzp_csi_rs_res_id_t>(cfg.rs);
  } else if (variant_holds_alternative<ssb_id_t>(cfg.rs)) {
    auto& ssb_idx = ploss_ref_rs.ref_sig.set_ssb_idx();
    ssb_idx       = variant_get<ssb_id_t>(cfg.rs);
  }
  return ploss_ref_rs;
}

asn1::rrc_nr::sri_pusch_pwr_ctrl_s
srsgnb::srs_du::make_asn1_rrc_sri_pusch_pwr_ctrl(const pusch_config::pusch_power_control::sri_pusch_pwr_ctrl& cfg)
{
  sri_pusch_pwr_ctrl_s sri_pwr_ctl;
  sri_pwr_ctl.sri_pusch_pwr_ctrl_id        = cfg.id;
  sri_pwr_ctl.sri_p0_pusch_alpha_set_id    = cfg.sri_p0_pusch_alphaset_id;
  sri_pwr_ctl.sri_pusch_pathloss_ref_rs_id = cfg.sri_pusch_pathloss_ref_rs_id;
  switch (cfg.closed_loop_idx) {
    case pusch_config::pusch_power_control::sri_pusch_pwr_ctrl::sri_pusch_closed_loop_index::i0:
      sri_pwr_ctl.sri_pusch_closed_loop_idx = sri_pusch_pwr_ctrl_s::sri_pusch_closed_loop_idx_opts::i0;
      break;
    case pusch_config::pusch_power_control::sri_pusch_pwr_ctrl::sri_pusch_closed_loop_index::i1:
      sri_pwr_ctl.sri_pusch_closed_loop_idx = sri_pusch_pwr_ctrl_s::sri_pusch_closed_loop_idx_opts::i1;
      break;
    default:
      srsgnb_assertion_failure("Invalid SRI Closed loop idx={}", cfg.closed_loop_idx);
  }
  return sri_pwr_ctl;
}

void make_asn1_rrc_alpha(asn1::rrc_nr::alpha_e& out, const alpha& cfg)
{
  switch (cfg) {
    case alpha::alpha0:
      out = alpha_opts::alpha0;
      break;
    case alpha::alpha04:
      out = alpha_opts::alpha04;
      break;
    case alpha::alpha05:
      out = alpha_opts::alpha05;
      break;
    case alpha::alpha06:
      out = alpha_opts::alpha06;
      break;
    case alpha::alpha07:
      out = alpha_opts::alpha07;
      break;
    case alpha::alpha08:
      out = alpha_opts::alpha08;
      break;
    case alpha::alpha09:
      out = alpha_opts::alpha09;
      break;
    case alpha::alpha1:
      out = alpha_opts::alpha1;
      break;
    default:
      srsgnb_assertion_failure("Invalid alpha={}", cfg);
  }
}

void make_asn1_rrc_pusch_pwr_ctrl(asn1::rrc_nr::pusch_pwr_ctrl_s&          out,
                                  const pusch_config::pusch_power_control& src,
                                  const pusch_config::pusch_power_control& dest)
{
  if (dest.is_tpc_accumulation_disabled) {
    out.tpc_accumulation_present = true;
  }

  if (dest.msg3_alpha != srsgnb::alpha::not_set) {
    out.msg3_alpha_present = true;
    make_asn1_rrc_alpha(out.msg3_alpha, dest.msg3_alpha);
  }

  if (dest.p0_nominal_without_grant.has_value()) {
    out.p0_nominal_without_grant_present = true;
    out.p0_nominal_without_grant         = dest.p0_nominal_without_grant.value();
  }

  for (unsigned idx = 0; idx < dest.p0_alphasets.size(); idx++) {
    p0_pusch_alpha_set_s p0_alphaset{};
    p0_alphaset.p0_pusch_alpha_set_id = dest.p0_alphasets[idx].id;
    if (dest.p0_alphasets[idx].p0.has_value()) {
      p0_alphaset.p0_present = true;
      p0_alphaset.p0         = dest.p0_alphasets[idx].p0.value();
    }
    if (dest.p0_alphasets[idx].p0_pusch_alpha != srsgnb::alpha::not_set) {
      p0_alphaset.alpha_present = true;
      make_asn1_rrc_alpha(p0_alphaset.alpha, dest.p0_alphasets[idx].p0_pusch_alpha);
    }
    out.p0_alpha_sets.push_back(p0_alphaset);
  }

  calculate_addmodremlist_diff(
      out.pathloss_ref_rs_to_add_mod_list,
      out.pathloss_ref_rs_to_release_list,
      src.pathloss_ref_rs,
      dest.pathloss_ref_rs,
      [](const pusch_config::pusch_power_control::pusch_pathloss_ref_rs& res) {
        return make_asn1_rrc_pusch_pathloss_ref_rs(res);
      },
      [](const pusch_config::pusch_power_control::pusch_pathloss_ref_rs& res) { return res.id; });

  calculate_addmodremlist_diff(
      out.sri_pusch_map_to_add_mod_list,
      out.sri_pusch_map_to_release_list,
      src.sri_pusch_mapping,
      dest.sri_pusch_mapping,
      [](const pusch_config::pusch_power_control::sri_pusch_pwr_ctrl& res) {
        return make_asn1_rrc_sri_pusch_pwr_ctrl(res);
      },
      [](const pusch_config::pusch_power_control::sri_pusch_pwr_ctrl& res) { return res.id; });
}

void calculate_pusch_config_diff(asn1::rrc_nr::pusch_cfg_s& out, const pusch_config& src, const pusch_config& dest)
{
  if (dest.data_scrambling_id_pusch.has_value()) {
    out.data_scrambling_id_pusch_present = true;
    out.data_scrambling_id_pusch         = dest.data_scrambling_id_pusch.value();
  }
  if (dest.tx_cfg != srsgnb::pusch_config::tx_config::not_set) {
    out.tx_cfg_present = true;
    switch (dest.tx_cfg) {
      case pusch_config::tx_config::codebook:
        out.tx_cfg = pusch_cfg_s::tx_cfg_opts::codebook;
        break;
      case pusch_config::tx_config::non_codebook:
        out.tx_cfg = pusch_cfg_s::tx_cfg_opts::non_codebook;
        break;
      default:
        srsgnb_assertion_failure("Invalid PUSCH Tx cfg={}", dest.tx_cfg);
    }
  }

  if ((dest.pusch_mapping_type_a_dmrs.has_value() && not src.pusch_mapping_type_a_dmrs.has_value()) ||
      (dest.pusch_mapping_type_a_dmrs.has_value() && src.pusch_mapping_type_a_dmrs.has_value() &&
       dest.pusch_mapping_type_a_dmrs != src.pusch_mapping_type_a_dmrs)) {
    out.dmrs_ul_for_pusch_map_type_a_present = true;
    make_asn1_rrc_dmrs_ul_for_pusch(out.dmrs_ul_for_pusch_map_type_a.set_setup(),
                                    src.pusch_mapping_type_a_dmrs.has_value() ? src.pusch_mapping_type_a_dmrs.value()
                                                                              : dmrs_uplink_config{},
                                    dest.pusch_mapping_type_a_dmrs.value());
  } else if (src.pusch_mapping_type_a_dmrs.has_value() && not dest.pusch_mapping_type_a_dmrs.has_value()) {
    out.dmrs_ul_for_pusch_map_type_a_present = true;
    out.dmrs_ul_for_pusch_map_type_a.set_release();
  }

  if ((dest.pusch_mapping_type_b_dmrs.has_value() && not src.pusch_mapping_type_b_dmrs.has_value()) ||
      (dest.pusch_mapping_type_b_dmrs.has_value() && src.pusch_mapping_type_b_dmrs.has_value() &&
       dest.pusch_mapping_type_b_dmrs != src.pusch_mapping_type_b_dmrs)) {
    out.dmrs_ul_for_pusch_map_type_b_present = true;
    make_asn1_rrc_dmrs_ul_for_pusch(out.dmrs_ul_for_pusch_map_type_b.set_setup(),
                                    src.pusch_mapping_type_b_dmrs.has_value() ? src.pusch_mapping_type_b_dmrs.value()
                                                                              : dmrs_uplink_config{},
                                    dest.pusch_mapping_type_b_dmrs.value());
  } else if (src.pusch_mapping_type_b_dmrs.has_value() && not dest.pusch_mapping_type_b_dmrs.has_value()) {
    out.dmrs_ul_for_pusch_map_type_b_present = true;
    out.dmrs_ul_for_pusch_map_type_b.set_release();
  }

  if (dest.pusch_pwr_ctrl.has_value()) {
    out.pusch_pwr_ctrl_present = true;
    make_asn1_rrc_pusch_pwr_ctrl(out.pusch_pwr_ctrl,
                                 src.pusch_pwr_ctrl.has_value() ? src.pusch_pwr_ctrl.value()
                                                                : pusch_config::pusch_power_control{},
                                 dest.pusch_pwr_ctrl.value());
  }

  switch (dest.res_alloc) {
    case pusch_config::resource_allocation::resource_allocation_type_0:
      out.res_alloc = pusch_cfg_s::res_alloc_opts::res_alloc_type0;
      break;
    case pusch_config::resource_allocation::resource_allocation_type_1:
      out.res_alloc = pusch_cfg_s::res_alloc_opts::res_alloc_type1;
      break;
    case pusch_config::resource_allocation::dynamic_switch:
      out.res_alloc = pusch_cfg_s::res_alloc_opts::dynamic_switch;
      break;
    default:
      srsgnb_assertion_failure("Invalid PUSCH Resource Allocation={}", dest.res_alloc);
  }

  if (dest.trans_precoder != srsgnb::pusch_config::transform_precoder::not_set) {
    out.transform_precoder_present = true;
    switch (dest.trans_precoder) {
      case pusch_config::transform_precoder::enabled:
        out.transform_precoder = pusch_cfg_s::transform_precoder_opts::enabled;
        break;
      case pusch_config::transform_precoder::disabled:
        out.transform_precoder = pusch_cfg_s::transform_precoder_opts::disabled;
        break;
      default:
        srsgnb_assertion_failure("Invalid PUSCH Transform Precoder={}", dest.trans_precoder);
    }
  }

  if (dest.cb_subset != srsgnb::pusch_config::codebook_subset::not_set) {
    out.codebook_subset_present = true;
    switch (dest.cb_subset) {
      case pusch_config::codebook_subset::fully_and_partial_and_non_coherent:
        out.codebook_subset = pusch_cfg_s::codebook_subset_opts::fully_and_partial_and_non_coherent;
        break;
      case pusch_config::codebook_subset::partial_and_non_coherent:
        out.codebook_subset = pusch_cfg_s::codebook_subset_opts::partial_and_non_coherent;
        break;
      case pusch_config::codebook_subset::non_coherent:
        out.codebook_subset = pusch_cfg_s::codebook_subset_opts::non_coherent;
        break;
      default:
        srsgnb_assertion_failure("Invalid Codebook subset={}", dest.cb_subset);
    }
  }

  if (dest.max_rank.has_value()) {
    out.max_rank_present = true;
    out.max_rank         = dest.max_rank.value();
  }
}

asn1::rrc_nr::srs_res_set_s srsgnb::srs_du::make_asn1_rrc_srs_res_set(const srs_config::srs_resource_set& cfg)
{
  srs_res_set_s srs_res_set;
  srs_res_set.srs_res_set_id = cfg.id;
  for (const auto& res_id : cfg.srs_res_id_list) {
    srs_res_set.srs_res_id_list.push_back(res_id);
  }

  if (variant_holds_alternative<srs_config::srs_resource_set::aperiodic_resource_type>(cfg.res_type)) {
    auto&       aper_res     = srs_res_set.res_type.set_aperiodic();
    const auto& cfg_aper_res = variant_get<srs_config::srs_resource_set::aperiodic_resource_type>(cfg.res_type);
    aper_res.aperiodic_srs_res_trigger = cfg_aper_res.aperiodic_srs_res_trigger;
    if (cfg_aper_res.csi_rs.has_value()) {
      aper_res.csi_rs_present = true;
      aper_res.csi_rs         = cfg_aper_res.csi_rs.value();
    }
    if (cfg_aper_res.slot_offset.has_value()) {
      aper_res.slot_offset_present = true;
      aper_res.slot_offset         = cfg_aper_res.slot_offset.value();
    }
    if (not cfg_aper_res.aperiodic_srs_res_trigger_list.empty()) {
      aper_res.aperiodic_srs_res_trigger_list.set_present();
      auto* aper_res_trig_list = aper_res.aperiodic_srs_res_trigger_list.get();
      for (const auto& ap_res_trig : cfg_aper_res.aperiodic_srs_res_trigger_list) {
        aper_res_trig_list->push_back(ap_res_trig);
      }
    }
  } else if (variant_holds_alternative<srs_config::srs_resource_set::semi_persistent_resource_type>(cfg.res_type)) {
    auto&       semi_p_res     = srs_res_set.res_type.set_semi_persistent();
    const auto& cfg_semi_p_res = variant_get<srs_config::srs_resource_set::semi_persistent_resource_type>(cfg.res_type);
    if (cfg_semi_p_res.associated_csi_rs.has_value()) {
      semi_p_res.associated_csi_rs_present = true;
      semi_p_res.associated_csi_rs         = cfg_semi_p_res.associated_csi_rs.value();
    }
  } else if (variant_holds_alternative<srs_config::srs_resource_set::periodic_resource_type>(cfg.res_type)) {
    auto&       per_res     = srs_res_set.res_type.set_periodic();
    const auto& cfg_per_res = variant_get<srs_config::srs_resource_set::periodic_resource_type>(cfg.res_type);
    if (cfg_per_res.associated_csi_rs.has_value()) {
      per_res.associated_csi_rs_present = true;
      per_res.associated_csi_rs         = cfg_per_res.associated_csi_rs.value();
    }
  }

  switch (cfg.srs_res_set_usage) {
    case srs_config::srs_resource_set::usage::beam_management:
      srs_res_set.usage = srs_res_set_s::usage_opts::beam_management;
      break;
    case srs_config::srs_resource_set::usage::codebook:
      srs_res_set.usage = srs_res_set_s::usage_opts::codebook;
      break;
    case srs_config::srs_resource_set::usage::non_codebook:
      srs_res_set.usage = srs_res_set_s::usage_opts::non_codebook;
      break;
    case srs_config::srs_resource_set::usage::antenna_switching:
      srs_res_set.usage = srs_res_set_s::usage_opts::ant_switching;
      break;
    default:
      srsgnb_assertion_failure("Invalid SRS resource set usage={}", cfg.srs_res_set_usage);
  }

  if (cfg.srs_pwr_ctrl_alpha != alpha::not_set) {
    srs_res_set.alpha_present = true;
    make_asn1_rrc_alpha(srs_res_set.alpha, cfg.srs_pwr_ctrl_alpha);
  }

  if (cfg.p0.has_value()) {
    srs_res_set.p0_present = true;
    srs_res_set.p0         = cfg.p0.value();
  }

  if (cfg.pwr_ctrl_adj_states != srs_config::srs_resource_set::srs_pwr_ctrl_adjustment_states::not_set) {
    srs_res_set.srs_pwr_ctrl_adjustment_states_present = true;
    switch (cfg.pwr_ctrl_adj_states) {
      case srs_config::srs_resource_set::srs_pwr_ctrl_adjustment_states::same_as_fci2:
        srs_res_set.srs_pwr_ctrl_adjustment_states = srs_res_set_s::srs_pwr_ctrl_adjustment_states_opts::same_as_fci2;
        break;
      case srs_config::srs_resource_set::srs_pwr_ctrl_adjustment_states::separate_closed_loop:
        srs_res_set.srs_pwr_ctrl_adjustment_states =
            srs_res_set_s::srs_pwr_ctrl_adjustment_states_opts::separate_closed_loop;
        break;
      default:
        srsgnb_assertion_failure("Invalid Power Control Adj. state={}", cfg.pwr_ctrl_adj_states);
    }
  }

  if (cfg.pathloss_ref_rs.has_value()) {
    srs_res_set.pathloss_ref_rs_present = true;
    if (variant_holds_alternative<ssb_id_t>(cfg.pathloss_ref_rs.value())) {
      auto& ssb_idx = srs_res_set.pathloss_ref_rs.set_ssb_idx();
      ssb_idx       = variant_get<ssb_id_t>(cfg.pathloss_ref_rs.value());
    } else if (variant_holds_alternative<nzp_csi_rs_res_id_t>(cfg.pathloss_ref_rs.value())) {
      auto& csi_res_idx = srs_res_set.pathloss_ref_rs.set_csi_rs_idx();
      csi_res_idx       = variant_get<nzp_csi_rs_res_id_t>(cfg.pathloss_ref_rs.value());
    }
  }

  return srs_res_set;
}

void calculate_srs_config_diff(asn1::rrc_nr::srs_cfg_s& out, const srs_config& src, const srs_config& dest)
{
  calculate_addmodremlist_diff(
      out.srs_res_set_to_add_mod_list,
      out.srs_res_set_to_release_list,
      src.srs_res_set,
      dest.srs_res_set,
      [](const srs_config::srs_resource_set& res) { return make_asn1_rrc_srs_res_set(res); },
      [](const srs_config::srs_resource_set& res) { return res.id; });

  // TODO SRS Resource

  if (dest.is_tpc_accum_disabled) {
    out.tpc_accumulation_present = true;
  }
}

void calculate_bwp_ul_dedicated_diff(asn1::rrc_nr::bwp_ul_ded_s& out,
                                     const bwp_uplink_dedicated& src,
                                     const bwp_uplink_dedicated& dest)
{
  if ((dest.pucch_cfg.has_value() && not src.pucch_cfg.has_value()) ||
      (dest.pucch_cfg.has_value() && src.pucch_cfg.has_value() && dest.pucch_cfg != src.pucch_cfg)) {
    out.pucch_cfg_present = true;
    calculate_pucch_config_diff(out.pucch_cfg.set_setup(),
                                src.pucch_cfg.has_value() ? src.pucch_cfg.value() : pucch_config{},
                                dest.pucch_cfg.value());
  } else if (src.pucch_cfg.has_value() && not dest.pucch_cfg.has_value()) {
    out.pucch_cfg_present = true;
    out.pucch_cfg.set_release();
  }

  if ((dest.pusch_cfg.has_value() && not src.pusch_cfg.has_value()) ||
      (dest.pusch_cfg.has_value() && src.pusch_cfg.has_value() && dest.pusch_cfg != src.pusch_cfg)) {
    out.pusch_cfg_present = true;
    calculate_pusch_config_diff(out.pusch_cfg.set_setup(),
                                src.pusch_cfg.has_value() ? src.pusch_cfg.value() : pusch_config{},
                                dest.pusch_cfg.value());
  } else if (src.pusch_cfg.has_value() && not dest.pusch_cfg.has_value()) {
    out.pusch_cfg_present = true;
    out.pusch_cfg.set_release();
  }

  if ((dest.srs_cfg.has_value() && not src.srs_cfg.has_value()) ||
      (dest.srs_cfg.has_value() && src.srs_cfg.has_value() && dest.srs_cfg != src.srs_cfg)) {
    out.srs_cfg_present = true;
    calculate_srs_config_diff(
        out.srs_cfg.set_setup(), src.srs_cfg.has_value() ? src.srs_cfg.value() : srs_config{}, dest.srs_cfg.value());
  } else if (src.srs_cfg.has_value() && not dest.srs_cfg.has_value()) {
    out.srs_cfg_present = true;
    out.srs_cfg.set_release();
  }
  // TODO: Remaining.
}

void calculate_uplink_config_diff(asn1::rrc_nr::ul_cfg_s& out, const uplink_config& src, const uplink_config& dest)
{
  out.init_ul_bwp_present = true;
  calculate_bwp_ul_dedicated_diff(out.init_ul_bwp, src.init_ul_bwp, dest.init_ul_bwp);

  // TODO: Remaining.
}

void calculate_serving_cell_config_diff(const serving_cell_config&        src,
                                        const serving_cell_config&        dest,
                                        asn1::rrc_nr::serving_cell_cfg_s& out)
{
  out.init_dl_bwp_present = true;
  calculate_bwp_dl_dedicated_diff(out.init_dl_bwp, src.init_dl_bwp, dest.init_dl_bwp);
  if (dest.ul_config.has_value()) {
    out.ul_cfg_present = true;
    calculate_uplink_config_diff(
        out.ul_cfg, src.ul_config.has_value() ? src.ul_config.value() : uplink_config{}, dest.ul_config.value());
  }
}

void srsgnb::srs_du::calculate_cell_group_config_diff(asn1::rrc_nr::cell_group_cfg_s& out,
                                                      const cell_group_config&        src,
                                                      const cell_group_config&        dest)
{
  calculate_addmodremlist_diff(
      out.rlc_bearer_to_add_mod_list,
      out.rlc_bearer_to_release_list,
      src.rlc_bearers,
      dest.rlc_bearers,
      [](const rlc_bearer_config& b) { return make_asn1_rrc_rlc_bearer(b); },
      [](const rlc_bearer_config& b) { return (uint8_t)b.lcid; });

  out.sp_cell_cfg_present                 = true;
  out.sp_cell_cfg.serv_cell_idx_present   = false;
  out.sp_cell_cfg.serv_cell_idx           = dest.spcell_cfg.serv_cell_idx;
  out.sp_cell_cfg.sp_cell_cfg_ded_present = true;
  calculate_serving_cell_config_diff(
      src.spcell_cfg.spcell_cfg_ded, dest.spcell_cfg.spcell_cfg_ded, out.sp_cell_cfg.sp_cell_cfg_ded);
}
