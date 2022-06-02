/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsgnb/phy/generic_functions/generic_functions_factories.h"
#include "dft_processor_fftw_impl.h"

namespace srsgnb {

class dft_processor_factory_fftw : public dft_processor_factory
{
  dft_processor_fftw_configuration fftw_config;

public:
  dft_processor_factory_fftw(const dft_processor_factory_fftw_configuration& common_config)
  {
    fftw_config.avoid_wisdom    = common_config.avoid_wisdom;
    fftw_config.wisdom_filename = common_config.wisdom_filename;
  }

private:
  std::unique_ptr<dft_processor> create(const dft_processor::configuration& dft_config) override
  {
    return std::make_unique<dft_processor_fftw_impl>(fftw_config, dft_config);
  }
};

} // namespace srsgnb

using namespace srsgnb;

std::shared_ptr<dft_processor_factory>
srsgnb::create_dft_processor_factory_fftw(const dft_processor_factory_fftw_configuration& common_config)
{
  return std::make_shared<dft_processor_factory_fftw>(common_config);
}
