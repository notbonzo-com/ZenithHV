#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limine.h>
#include <vmm.hpp>

namespace vmm
{

extern struct limine_kernel_address_response *kernel_address;

void init();
void setCTX(const page_map_ctx* pmc);


} // namespace vmm
