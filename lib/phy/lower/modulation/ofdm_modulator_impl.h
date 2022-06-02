/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef LIB_PHY_LOWER_MODULATION_OFDM_MODULATOR_IMPL_H
#define LIB_PHY_LOWER_MODULATION_OFDM_MODULATOR_IMPL_H

#include "srsgnb/phy/cyclic_prefix.h"
#include "srsgnb/phy/generic_functions/dft_processor.h"
#include "srsgnb/phy/lower/modulation/ofdm_modulator.h"
#include "srsgnb/phy/resource_grid.h"
#include "srsgnb/srsvec/aligned_vec.h"

namespace srsgnb {

/// OFDM modulator factory configuration. Provides the necessary dependency instances.
struct ofdm_modulator_common_configuration {
  /// DFT instance. The ownership is transferred to the modulator.
  std::unique_ptr<dft_processor> dft;
};

/// Describes a generic OFDM symbol modulator.
class ofdm_symbol_modulator_impl : public ofdm_symbol_modulator
{
private:
  /// Indicates the DFT size.
  unsigned dft_size;
  /// Indicates the resource grid bandwidth in resource elements.
  unsigned rg_size;
  /// Indicates the cyclic prefix length.
  cyclic_prefix cp;
  /// Indicates the numerology.
  unsigned numerology;
  /// Indicates the scaling factor at the DFT output.
  float scale;
  /// Indicates the center frequency of the carrier in Hz.
  double center_freq_hz;
  /// DFT processor.
  std::unique_ptr<dft_processor> dft;

  /// \brief Gets the offset to a symbol including the cyclic prefixes.
  /// \param[in] symbol_index Indicates the symbol index within the subframe.
  /// \return The number of samples to the start of the given symbol.
  unsigned get_symbol_offset(unsigned symbol_index) const;

  /// \brief Computes the phase compensation (TS 138.211, Section 5.4) for a given symbol.
  /// \param[in] symbol_index Indicates the symbol index within the subframe.
  /// \return The phase compensation to be applied to the given symbol.
  cf_t get_phase_compensation(unsigned symbol_index) const;

public:
  /// \brief Constructs an OFDM symbol modulator.
  /// \param[in] common_config Provides specific configuration parameters from the factory.
  /// \param[in] ofdm_config Provides generic OFDM configuration parameters.
  ofdm_symbol_modulator_impl(ofdm_modulator_common_configuration& common_config,
                             const ofdm_modulator_configuration&  ofdm_config);

  // See interface for documentation.
  unsigned get_symbol_size(unsigned symbol_index) const override
  {
    return cp.get_length(symbol_index, numerology, dft_size) + dft_size;
  }

  // See interface for documentation.
  void
  modulate(span<cf_t> ouput, const resource_grid_reader& grid, unsigned port_index, unsigned symbol_index) override;
};

/// Describes a generic OFDM slot modulator.
class ofdm_slot_modulator_impl : public ofdm_slot_modulator
{
private:
  /// Indicates the cyclic prefix length.
  cyclic_prefix cp;
  /// Instance of symbol modulator.
  ofdm_symbol_modulator_impl symbol_modulator;

public:
  /// \brief Constructs an OFDM slot modulator.
  /// \param[in] common_config Provides specific configuration parameters from the factory.
  /// \param[in] ofdm_config Provides generic OFDM configuration parameters.
  ofdm_slot_modulator_impl(ofdm_modulator_common_configuration& common_config,
                           const ofdm_modulator_configuration&  ofdm_config) :
    cp(ofdm_config.cp), symbol_modulator(common_config, ofdm_config)
  {
    // Do nothing.
  }

  // See interface for documentation;
  unsigned get_slot_size(unsigned slot_index) const override;

  // See interface for documentation;
  void modulate(span<cf_t> output, const resource_grid_reader& grid, unsigned port_index, unsigned slot_index) override;
};

} // namespace srsgnb

#endif // LIB_PHY_LOWER_MODULATION_OFDM_MODULATOR_IMPL_H
