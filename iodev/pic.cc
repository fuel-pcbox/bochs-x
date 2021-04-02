/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2002-2021  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
/////////////////////////////////////////////////////////////////////////

// Define BX_PLUGGABLE in files that can be compiled into plugins.  For
// platforms that require a special tag on exported symbols, BX_PLUGGABLE
// is used to know when we are exporting symbols and when we are importing.
#define BX_PLUGGABLE

#include "iodev.h"
#include "pic.h"

#define LOG_THIS thePic->

bx_pic_c *thePic = NULL;

PLUGIN_ENTRY_FOR_MODULE(pic)
{
  if (mode == PLUGIN_INIT) {
    thePic = new bx_pic_c();
    bx_devices.pluginPicDevice = thePic;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, thePic, BX_PLUGIN_PIC);
  } else if (mode == PLUGIN_FINI) {
    delete thePic;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_CORE;
  }
  return 0; // Success
}

bx_pic_c::bx_pic_c(void)
{
  put("PIC");
}

bx_pic_c::~bx_pic_c(void)
{
  SIM->get_bochs_root()->remove("pic");
  BX_DEBUG(("Exit"));
}

void bx_pic_c::init(void)
{
  /* 8259 PIC (Programmable Interrupt Controller) */
  DEV_register_ioread_handler(this, read_handler, 0x0020, "8259 PIC", 1);
  DEV_register_ioread_handler(this, read_handler, 0x0021, "8259 PIC", 1);
  DEV_register_ioread_handler(this, read_handler, 0x00A0, "8259 PIC", 1);
  DEV_register_ioread_handler(this, read_handler, 0x00A1, "8259 PIC", 1);

  DEV_register_iowrite_handler(this, write_handler, 0x0020, "8259 PIC", 1);
  DEV_register_iowrite_handler(this, write_handler, 0x0021, "8259 PIC", 1);
  DEV_register_iowrite_handler(this, write_handler, 0x00A0, "8259 PIC", 1);
  DEV_register_iowrite_handler(this, write_handler, 0x00A1, "8259 PIC", 1);


  BX_PIC_THIS s.master_pic.master = 1;
  BX_PIC_THIS s.master_pic.interrupt_offset = 0x08; /* IRQ0 = INT 0x08 */
  BX_PIC_THIS s.master_pic.sfnm = 0; /* normal nested mode */
  BX_PIC_THIS s.master_pic.buffered_mode = 0; /* unbuffered mode */
  BX_PIC_THIS s.master_pic.master_slave  = 1; /* master PIC */
  BX_PIC_THIS s.master_pic.auto_eoi      = 0; /* manual EOI from CPU */
  BX_PIC_THIS s.master_pic.imr           = 0xFF; /* all IRQ's initially masked */
  BX_PIC_THIS s.master_pic.isr           = 0x00; /* no IRQ's in service */
  BX_PIC_THIS s.master_pic.irr           = 0x00; /* no IRQ's requested */
  BX_PIC_THIS s.master_pic.read_reg_select = 0; /* IRR */
  BX_PIC_THIS s.master_pic.irq = 0;
  BX_PIC_THIS s.master_pic.INT = 0;
  BX_PIC_THIS s.master_pic.init.in_init = 0;
  BX_PIC_THIS s.master_pic.init.requires_4 = 0;
  BX_PIC_THIS s.master_pic.init.byte_expected = 0;
  BX_PIC_THIS s.master_pic.special_mask = 0;
  BX_PIC_THIS s.master_pic.lowest_priority = 7;
  BX_PIC_THIS s.master_pic.polled = 0;
  BX_PIC_THIS s.master_pic.rotate_on_autoeoi = 0;
  BX_PIC_THIS s.master_pic.edge_level = 0;
  BX_PIC_THIS s.master_pic.IRQ_in = 0;

  BX_PIC_THIS s.slave_pic.master = 0;
  BX_PIC_THIS s.slave_pic.interrupt_offset = 0x70; /* IRQ8 = INT 0x70 */
  BX_PIC_THIS s.slave_pic.sfnm       = 0; /* normal nested mode */
  BX_PIC_THIS s.slave_pic.buffered_mode = 0; /* unbuffered mode */
  BX_PIC_THIS s.slave_pic.master_slave  = 0; /* slave PIC */
  BX_PIC_THIS s.slave_pic.auto_eoi      = 0; /* manual EOI from CPU */
  BX_PIC_THIS s.slave_pic.imr           = 0xFF; /* all IRQ's initially masked */
  BX_PIC_THIS s.slave_pic.isr           = 0x00; /* no IRQ's in service */
  BX_PIC_THIS s.slave_pic.irr           = 0x00; /* no IRQ's requested */
  BX_PIC_THIS s.slave_pic.read_reg_select = 0; /* IRR */
  BX_PIC_THIS s.slave_pic.irq = 0;
  BX_PIC_THIS s.slave_pic.INT = 0;
  BX_PIC_THIS s.slave_pic.init.in_init = 0;
  BX_PIC_THIS s.slave_pic.init.requires_4 = 0;
  BX_PIC_THIS s.slave_pic.init.byte_expected = 0;
  BX_PIC_THIS s.slave_pic.special_mask = 0;
  BX_PIC_THIS s.slave_pic.lowest_priority = 7;
  BX_PIC_THIS s.slave_pic.polled = 0;
  BX_PIC_THIS s.slave_pic.rotate_on_autoeoi = 0;
  BX_PIC_THIS s.slave_pic.edge_level = 0;
  BX_PIC_THIS s.slave_pic.IRQ_in = 0;

#if BX_DEBUGGER
  // register device for the 'info device' command (calls debug_dump())
  bx_dbg_register_debug_info("pic", this);
#endif
}

void bx_pic_c::reset(unsigned type) {}

void bx_pic_c::register_state(void)
{
  bx_list_c *ctrl;

  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "pic", "PIC State");
  ctrl = new bx_list_c(list, "master");
  new bx_shadow_num_c(ctrl, "interrupt_offset", &BX_PIC_THIS s.master_pic.interrupt_offset, BASE_HEX);
  new bx_shadow_num_c(ctrl, "auto_eoi", &BX_PIC_THIS s.master_pic.auto_eoi, BASE_HEX);
  new bx_shadow_num_c(ctrl, "imr", &BX_PIC_THIS s.master_pic.imr, BASE_HEX);
  new bx_shadow_num_c(ctrl, "isr", &BX_PIC_THIS s.master_pic.isr, BASE_HEX);
  new bx_shadow_num_c(ctrl, "irr", &BX_PIC_THIS s.master_pic.irr, BASE_HEX);
  new bx_shadow_num_c(ctrl, "read_reg_select", &BX_PIC_THIS s.master_pic.read_reg_select);
  new bx_shadow_num_c(ctrl, "irq", &BX_PIC_THIS s.master_pic.irq, BASE_HEX);
  new bx_shadow_num_c(ctrl, "lowest_priority", &BX_PIC_THIS s.master_pic.lowest_priority, BASE_HEX);
  BXRS_PARAM_BOOL(ctrl, INT, BX_PIC_THIS s.master_pic.INT);
  new bx_shadow_num_c(ctrl, "IRQ_in", &BX_PIC_THIS s.master_pic.IRQ_in, BASE_HEX);
  BXRS_PARAM_BOOL(ctrl, in_init, BX_PIC_THIS s.master_pic.init.in_init);
  BXRS_PARAM_BOOL(ctrl, requires_4, BX_PIC_THIS s.master_pic.init.requires_4);
  new bx_shadow_num_c(ctrl, "byte_expected", &BX_PIC_THIS s.master_pic.init.byte_expected);
  BXRS_PARAM_BOOL(ctrl, special_mask, BX_PIC_THIS s.master_pic.special_mask);
  BXRS_PARAM_BOOL(ctrl, polled, BX_PIC_THIS s.master_pic.polled);
  BXRS_PARAM_BOOL(ctrl, rotate_on_autoeoi, BX_PIC_THIS s.master_pic.rotate_on_autoeoi);
  new bx_shadow_num_c(ctrl, "edge_level", &BX_PIC_THIS s.master_pic.edge_level, BASE_HEX);
  ctrl = new bx_list_c(list, "slave");
  new bx_shadow_num_c(ctrl, "interrupt_offset", &BX_PIC_THIS s.slave_pic.interrupt_offset, BASE_HEX);
  new bx_shadow_num_c(ctrl, "auto_eoi", &BX_PIC_THIS s.slave_pic.auto_eoi, BASE_HEX);
  new bx_shadow_num_c(ctrl, "imr", &BX_PIC_THIS s.slave_pic.imr, BASE_HEX);
  new bx_shadow_num_c(ctrl, "isr", &BX_PIC_THIS s.slave_pic.isr, BASE_HEX);
  new bx_shadow_num_c(ctrl, "irr", &BX_PIC_THIS s.slave_pic.irr, BASE_HEX);
  new bx_shadow_num_c(ctrl, "read_reg_select", &BX_PIC_THIS s.slave_pic.read_reg_select);
  new bx_shadow_num_c(ctrl, "irq", &BX_PIC_THIS s.slave_pic.irq, BASE_HEX);
  new bx_shadow_num_c(ctrl, "lowest_priority", &BX_PIC_THIS s.slave_pic.lowest_priority, BASE_HEX);
  BXRS_PARAM_BOOL(ctrl, INT, BX_PIC_THIS s.slave_pic.INT);
  new bx_shadow_num_c(ctrl, "IRQ_in", &BX_PIC_THIS s.slave_pic.IRQ_in, BASE_HEX);
  BXRS_PARAM_BOOL(ctrl, in_init, BX_PIC_THIS s.slave_pic.init.in_init);
  BXRS_PARAM_BOOL(ctrl, requires_4, BX_PIC_THIS s.slave_pic.init.requires_4);
  new bx_shadow_num_c(ctrl, "byte_expected", &BX_PIC_THIS s.slave_pic.init.byte_expected);
  BXRS_PARAM_BOOL(ctrl, special_mask, BX_PIC_THIS s.slave_pic.special_mask);
  BXRS_PARAM_BOOL(ctrl, polled, BX_PIC_THIS s.slave_pic.polled);
  BXRS_PARAM_BOOL(ctrl, rotate_on_autoeoi, BX_PIC_THIS s.slave_pic.rotate_on_autoeoi);
  new bx_shadow_num_c(ctrl, "edge_level", &BX_PIC_THIS s.slave_pic.edge_level, BASE_HEX);
}

// static IO port read callback handler
// redirects to non-static class handler to avoid virtual functions
Bit32u bx_pic_c::read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_PIC_SMF
  bx_pic_c *class_ptr = (bx_pic_c *) this_ptr;
  return class_ptr->read(address, io_len);
}

Bit32u bx_pic_c::read(Bit32u address, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_PIC_SMF

  BX_DEBUG(("IO read from 0x%04x", address));

  /*
   8259A PIC
   */

  if((address == 0x20 || address == 0x21) && BX_PIC_THIS s.master_pic.polled) {
    // In polled mode. Treat this as an interrupt acknowledge
    clear_highest_interrupt(& BX_PIC_THIS s.master_pic);
    BX_PIC_THIS s.master_pic.polled = 0;
    pic_service(& BX_PIC_THIS s.master_pic);
    return io_len==1?BX_PIC_THIS s.master_pic.irq:(BX_PIC_THIS s.master_pic.irq)<<8|(BX_PIC_THIS s.master_pic.irq);  // Return the current irq requested
  }

  if((address == 0xa0 || address == 0xa1) && BX_PIC_THIS s.slave_pic.polled) {
    // In polled mode. Treat this as an interrupt acknowledge
    clear_highest_interrupt(& BX_PIC_THIS s.slave_pic);
    BX_PIC_THIS s.slave_pic.polled = 0;
    pic_service(& BX_PIC_THIS s.slave_pic);
    return io_len==1?BX_PIC_THIS s.slave_pic.irq:(BX_PIC_THIS s.slave_pic.irq)<<8|(BX_PIC_THIS s.slave_pic.irq);  // Return the current irq requested
  }

  switch (address) {
    case 0x20:
      if (BX_PIC_THIS s.master_pic.read_reg_select) { /* ISR */
        BX_DEBUG(("read master ISR = 0x%02x", BX_PIC_THIS s.master_pic.isr));
        return BX_PIC_THIS s.master_pic.isr;
      } else { /* IRR */
        BX_DEBUG(("read master IRR = 0x%02x", BX_PIC_THIS s.master_pic.irr));
        return BX_PIC_THIS s.master_pic.irr;
      }
      break;
    case 0x21:
      BX_DEBUG(("read master IMR = 0x%02x", BX_PIC_THIS s.master_pic.imr));
      return BX_PIC_THIS s.master_pic.imr;
    case 0xA0:
      if (BX_PIC_THIS s.slave_pic.read_reg_select) { /* ISR */
        BX_DEBUG(("read slave ISR = 0x%02x", BX_PIC_THIS s.slave_pic.isr));
        return BX_PIC_THIS s.slave_pic.isr;
      } else { /* IRR */
        BX_DEBUG(("read slave IRR = 0x%02x", BX_PIC_THIS s.slave_pic.irr));
        return BX_PIC_THIS s.slave_pic.irr;
      }
      break;
    case 0xA1:
      BX_DEBUG(("read slave IMR = 0x%02x", BX_PIC_THIS s.slave_pic.imr));
      return BX_PIC_THIS s.slave_pic.imr;
  }

  BX_PANIC(("io read to address 0x%04x", address));
  return 0; /* default if not found above */
}


// static IO port write callback handler
// redirects to non-static class handler to avoid virtual functions

void bx_pic_c::write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_PIC_SMF
  bx_pic_c *class_ptr = (bx_pic_c *) this_ptr;
  class_ptr->write(address, value, io_len);
}

void bx_pic_c::write(Bit32u address, Bit32u value, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_PIC_SMF

  BX_DEBUG(("IO write to 0x%04x = 0x%02x", address, (Bit8u)value));

  /*
   8259A PIC
   */

  switch (address) {
    case 0x20:
      if (value & 0x10) { /* initialization command 1 */
        BX_DEBUG(("master: ICW1 found"));
        BX_DEBUG(("        requires 4 = %u", (value & 0x01)));
        if (value & 0x02) {
          BX_PANIC(("        single mode not supported"));
        } else {
          BX_DEBUG(("        cascade mode selected"));
        }
        if (value & 0x08) {
          BX_PANIC(("        level sensitive mode not supported"));
        } else {
          BX_DEBUG(("        edge triggered mode selected"));
        }
        BX_PIC_THIS s.master_pic.init.in_init = 1;
        BX_PIC_THIS s.master_pic.init.requires_4 = (value & 0x01);
        BX_PIC_THIS s.master_pic.init.byte_expected = 2; /* operation command 2 */
        BX_PIC_THIS s.master_pic.imr           = 0x00; /* clear the irq mask register */
        BX_PIC_THIS s.master_pic.isr           = 0x00; /* no IRQ's in service */
        BX_PIC_THIS s.master_pic.irr           = 0x00; /* no IRQ's requested */
        BX_PIC_THIS s.master_pic.lowest_priority = 7;
        BX_PIC_THIS s.master_pic.INT = 0; /* reprogramming clears previous INTR request */
        BX_PIC_THIS s.master_pic.auto_eoi = 0;
        BX_PIC_THIS s.master_pic.rotate_on_autoeoi = 0;
        BX_CLEAR_INTR();
        return;
      }

      if ((value & 0x18) == 0x08) { /* OCW3 */
        Bit8u special_mask, poll, read_op;

        special_mask = (value & 0x60) >> 5;
        poll         = (value & 0x04) >> 2;
        read_op      = (value & 0x03);
        if (poll) {
          BX_PIC_THIS s.master_pic.polled = 1;
          return;
        }
        if (read_op == 0x02) /* read IRR */
         BX_PIC_THIS s.master_pic.read_reg_select = 0;
        else if (read_op == 0x03) /* read ISR */
         BX_PIC_THIS s.master_pic.read_reg_select = 1;
        if (special_mask == 0x02) { /* cancel special mask */
          BX_PIC_THIS s.master_pic.special_mask = 0;
        }
        else if (special_mask == 0x03) { /* set specific mask */
          BX_PIC_THIS s.master_pic.special_mask = 1;
          pic_service(& BX_PIC_THIS s.master_pic);
        }
        return;
      }

      /* OCW2 */
      switch (value) {
        case 0x00: // Rotate in auto eoi mode clear
        case 0x80: // Rotate in auto eoi mode set
          BX_PIC_THIS s.master_pic.rotate_on_autoeoi = (value != 0);
          break;

        case 0xA0: // Rotate on non-specific end of interrupt
        case 0x20: /* end of interrupt command */
          clear_highest_interrupt(& BX_PIC_THIS s.master_pic);

          if(value == 0xA0) {// Rotate in Auto-EOI mode
            BX_PIC_THIS s.master_pic.lowest_priority ++;
            if(BX_PIC_THIS s.master_pic.lowest_priority > 7)
              BX_PIC_THIS s.master_pic.lowest_priority = 0;
          }

          pic_service(& BX_PIC_THIS s.master_pic);
          break;

        case 0x40: // Intel PIC spec-sheet seems to indicate this should be ignored
          BX_INFO(("IRQ no-op"));
          break;

        case 0x60: /* specific EOI 0 */
        case 0x61: /* specific EOI 1 */
        case 0x62: /* specific EOI 2 */
        case 0x63: /* specific EOI 3 */
        case 0x64: /* specific EOI 4 */
        case 0x65: /* specific EOI 5 */
        case 0x66: /* specific EOI 6 */
        case 0x67: /* specific EOI 7 */
          BX_PIC_THIS s.master_pic.isr &= ~(1 << (value-0x60));
          pic_service(& BX_PIC_THIS s.master_pic);
          break;

        // IRQ lowest priority commands
        case 0xC0: // 0 7 6 5 4 3 2 1
        case 0xC1: // 1 0 7 6 5 4 3 2
        case 0xC2: // 2 1 0 7 6 5 4 3
        case 0xC3: // 3 2 1 0 7 6 5 4
        case 0xC4: // 4 3 2 1 0 7 6 5
        case 0xC5: // 5 4 3 2 1 0 7 6
        case 0xC6: // 6 5 4 3 2 1 0 7
        case 0xC7: // 7 6 5 4 3 2 1 0
          BX_INFO(("IRQ lowest command 0x%x", value));
          BX_PIC_THIS s.master_pic.lowest_priority = value - 0xC0;
          break;

        case 0xE0: // specific EOI and rotate 0
        case 0xE1: // specific EOI and rotate 1
        case 0xE2: // specific EOI and rotate 2
        case 0xE3: // specific EOI and rotate 3
        case 0xE4: // specific EOI and rotate 4
        case 0xE5: // specific EOI and rotate 5
        case 0xE6: // specific EOI and rotate 6
        case 0xE7: // specific EOI and rotate 7
          BX_PIC_THIS s.master_pic.isr &= ~(1 << (value-0xE0));
          BX_PIC_THIS s.master_pic.lowest_priority = (value - 0xE0);
          pic_service(& BX_PIC_THIS s.master_pic);
          break;

        case 0x02: // single mode bit: 1 = single, 0 = cascade
          // ignore. 386BSD writes this value but works with it ignored.
          break;

        default:
          BX_PANIC(("write to port 20h = %02x", value));
      } /* switch (value) */
      break;

    case 0x21:
      /* initialization mode operation */
      if (BX_PIC_THIS s.master_pic.init.in_init) {
        switch (BX_PIC_THIS s.master_pic.init.byte_expected) {
          case 2:
            BX_PIC_THIS s.master_pic.interrupt_offset = value & 0xf8;
            BX_PIC_THIS s.master_pic.init.byte_expected = 3;
            BX_DEBUG(("master: ICW2 received"));
            BX_DEBUG(("        offset = INT %02x",
                      BX_PIC_THIS s.master_pic.interrupt_offset));
            break;
          case 3:
            BX_DEBUG(("master: ICW3 received"));
            if (value == 0x04) {
              BX_DEBUG(("        slave PIC on IRQ line #2"));
            } else {
              BX_PANIC(("        slave PIC IRQ line not supported"));
            }
            if (BX_PIC_THIS s.master_pic.init.requires_4) {
              BX_PIC_THIS s.master_pic.init.byte_expected = 4;
            } else {
              BX_PIC_THIS s.master_pic.init.in_init = 0;
            }
            break;
          case 4:
            BX_DEBUG(("master: ICW4 received"));
            if (value & 0x02) {
              BX_DEBUG(("        auto EOI"));
              BX_PIC_THIS s.master_pic.auto_eoi = 1;
            } else {
              BX_DEBUG(("        normal EOI interrupt"));
              BX_PIC_THIS s.master_pic.auto_eoi = 0;
            }
            if (value & 0x01) {
              BX_DEBUG(("        80x86 mode"));
            } else {
              BX_PANIC(("        not 80x86 mode"));
            }
            BX_PIC_THIS s.master_pic.init.in_init = 0;
            break;
          default:
            BX_PANIC(("master expecting bad init command"));
        }
        return;
      }

      /* normal operation */
      BX_DEBUG(("setting master pic IMR to %02x", value));
      BX_PIC_THIS s.master_pic.imr = value;
      pic_service(& BX_PIC_THIS s.master_pic);
      return;

    case 0xA0:
      if (value & 0x10) { /* initialization command 1 */
        BX_DEBUG(("slave:  ICW1 found"));
        BX_DEBUG(("        requires 4 = %u", (value & 0x01)));
        if (value & 0x02) {
          BX_PANIC(("        single mode not supported"));
        } else {
          BX_DEBUG(("        cascade mode selected"));
        }
        if (value & 0x08) {
          BX_PANIC(("        level sensitive mode not supported"));
        } else {
          BX_DEBUG(("        edge triggered mode selected"));
        }
        BX_PIC_THIS s.slave_pic.init.in_init = 1;
        BX_PIC_THIS s.slave_pic.init.requires_4 = (value & 0x01);
        BX_PIC_THIS s.slave_pic.init.byte_expected = 2; /* operation command 2 */
        BX_PIC_THIS s.slave_pic.imr           = 0x00; /* clear irq mask */
        BX_PIC_THIS s.slave_pic.isr           = 0x00; /* no IRQ's in service */
        BX_PIC_THIS s.slave_pic.irr           = 0x00; /* no IRQ's requested */
        BX_PIC_THIS s.slave_pic.lowest_priority = 7;
        BX_PIC_THIS s.slave_pic.INT = 0; /* reprogramming clears previous INTR request */
        BX_PIC_THIS s.master_pic.IRQ_in &= ~(1 << 2);
        BX_PIC_THIS s.slave_pic.auto_eoi = 0;
        BX_PIC_THIS s.slave_pic.rotate_on_autoeoi = 0;
        return;
      }

      if ((value & 0x18) == 0x08) { /* OCW3 */
        Bit8u special_mask, poll, read_op;

        special_mask = (value & 0x60) >> 5;
        poll         = (value & 0x04) >> 2;
        read_op      = (value & 0x03);
        if (poll) {
          BX_PIC_THIS s.slave_pic.polled = 1;
          return;
        }
        if (read_op == 0x02) /* read IRR */
         BX_PIC_THIS s.slave_pic.read_reg_select = 0;
        else if (read_op == 0x03) /* read ISR */
         BX_PIC_THIS s.slave_pic.read_reg_select = 1;
        if (special_mask == 0x02) { /* cancel special mask */
          BX_PIC_THIS s.slave_pic.special_mask = 0;
        }
        else if (special_mask == 0x03) { /* set specific mask */
          BX_PIC_THIS s.slave_pic.special_mask = 1;
          pic_service(& BX_PIC_THIS s.slave_pic);
        }
        return;
      }

      switch (value) {
        case 0x00: // Rotate in auto eoi mode clear
        case 0x80: // Rotate in auto eoi mode set
          BX_PIC_THIS s.slave_pic.rotate_on_autoeoi = (value != 0);
          break;

        case 0xA0: // Rotate on non-specific end of interrupt
        case 0x20: /* end of interrupt command */
          clear_highest_interrupt(& BX_PIC_THIS s.slave_pic);

          if(value == 0xA0) {// Rotate in Auto-EOI mode
            BX_PIC_THIS s.slave_pic.lowest_priority ++;
            if(BX_PIC_THIS s.slave_pic.lowest_priority > 7)
              BX_PIC_THIS s.slave_pic.lowest_priority = 0;
          }

          pic_service(& BX_PIC_THIS s.slave_pic);
          break;

        case 0x40: // Intel PIC spec-sheet seems to indicate this should be ignored
          BX_INFO(("IRQ no-op"));
          break;

        case 0x60: /* specific EOI 0 */
        case 0x61: /* specific EOI 1 */
        case 0x62: /* specific EOI 2 */
        case 0x63: /* specific EOI 3 */
        case 0x64: /* specific EOI 4 */
        case 0x65: /* specific EOI 5 */
        case 0x66: /* specific EOI 6 */
        case 0x67: /* specific EOI 7 */
          BX_PIC_THIS s.slave_pic.isr &= ~(1 << (value-0x60));
          pic_service(& BX_PIC_THIS s.slave_pic);
          break;

        // IRQ lowest priority commands
        case 0xC0: // 0 7 6 5 4 3 2 1
        case 0xC1: // 1 0 7 6 5 4 3 2
        case 0xC2: // 2 1 0 7 6 5 4 3
        case 0xC3: // 3 2 1 0 7 6 5 4
        case 0xC4: // 4 3 2 1 0 7 6 5
        case 0xC5: // 5 4 3 2 1 0 7 6
        case 0xC6: // 6 5 4 3 2 1 0 7
        case 0xC7: // 7 6 5 4 3 2 1 0
          BX_INFO(("IRQ lowest command 0x%x", value));
          BX_PIC_THIS s.slave_pic.lowest_priority = value - 0xC0;
          break;

        case 0xE0: // specific EOI and rotate 0
        case 0xE1: // specific EOI and rotate 1
        case 0xE2: // specific EOI and rotate 2
        case 0xE3: // specific EOI and rotate 3
        case 0xE4: // specific EOI and rotate 4
        case 0xE5: // specific EOI and rotate 5
        case 0xE6: // specific EOI and rotate 6
        case 0xE7: // specific EOI and rotate 7
          BX_PIC_THIS s.slave_pic.isr &= ~(1 << (value-0xE0));
          BX_PIC_THIS s.slave_pic.lowest_priority = (value - 0xE0);
          pic_service(& BX_PIC_THIS s.slave_pic);
          break;

        case 0x02: // single mode bit: 1 = single, 0 = cascade
          // ignore. 386BSD writes this value but works with it ignored.
          break;

        default:
          BX_PANIC(("write to port A0h = %02x", value));
      } /* switch (value) */
      break;

    case 0xA1:
      /* initialization mode operation */
      if (BX_PIC_THIS s.slave_pic.init.in_init) {
        switch (BX_PIC_THIS s.slave_pic.init.byte_expected) {
          case 2:
            BX_PIC_THIS s.slave_pic.interrupt_offset = value & 0xf8;
            BX_PIC_THIS s.slave_pic.init.byte_expected = 3;
            BX_DEBUG(("slave:  ICW2 received"));
            BX_DEBUG(("        offset = INT %02x",
                      BX_PIC_THIS s.slave_pic.interrupt_offset));
            break;
          case 3:
            BX_DEBUG(("slave:  ICW3 received"));
            if ((value & 0x07) == 0x02) {
              BX_DEBUG(("        slave PIC ID = 2"));
            } else {
              BX_PANIC(("        slave PIC ID = %d not supported", value & 0x07));
            }
            if (BX_PIC_THIS s.slave_pic.init.requires_4) {
              BX_PIC_THIS s.slave_pic.init.byte_expected = 4;
            } else {
              BX_PIC_THIS s.slave_pic.init.in_init = 0;
            }
            break;
          case 4:
            BX_DEBUG(("slave:  ICW4 received"));
            if (value & 0x02) {
              BX_DEBUG(("        auto EOI"));
              BX_PIC_THIS s.slave_pic.auto_eoi = 1;
            } else {
              BX_DEBUG(("        normal EOI interrupt"));
              BX_PIC_THIS s.slave_pic.auto_eoi = 0;
            }
            if (value & 0x01) {
              BX_DEBUG(("        80x86 mode"));
            } else {
              BX_PANIC(("        not 80x86 mode"));
            }
            BX_PIC_THIS s.slave_pic.init.in_init = 0;
            break;
          default:
            BX_PANIC(("slave:  expecting bad init command"));
        }
        return;
      }

      /* normal operation */
      BX_DEBUG(("setting slave pic IMR to %02x", value));
      BX_PIC_THIS s.slave_pic.imr = value;
      pic_service(& BX_PIC_THIS s.slave_pic);
  } /* switch (address) */
}

// new IRQ signal handling routines

void bx_pic_c::lower_irq(unsigned irq_no)
{
#if BX_SUPPORT_APIC
  // forward this function call to the ioapic too
  if (DEV_ioapic_present() && (irq_no != 2)) {
    DEV_ioapic_set_irq_level(irq_no, 0);
  }
#endif

  Bit8u mask = (1 << (irq_no & 7));
  if ((irq_no <= 7) && (BX_PIC_THIS s.master_pic.IRQ_in & mask)) {
    BX_DEBUG(("IRQ line %d now low", irq_no));
    BX_PIC_THIS s.master_pic.IRQ_in &= ~(mask);
    BX_PIC_THIS s.master_pic.irr &= ~(mask);
  } else if ((irq_no > 7) && (irq_no <= 15) &&
             (BX_PIC_THIS s.slave_pic.IRQ_in & mask)) {
    BX_DEBUG(("IRQ line %d now low", irq_no));
    BX_PIC_THIS s.slave_pic.IRQ_in &= ~(mask);
    BX_PIC_THIS s.slave_pic.irr &= ~(mask);
  }
}

void bx_pic_c::raise_irq(unsigned irq_no)
{
#if BX_SUPPORT_APIC
  // forward this function call to the ioapic too
  if (DEV_ioapic_present() && (irq_no != 2)) {
    DEV_ioapic_set_irq_level(irq_no, 1);
  }
#endif

  Bit8u mask = (1 << (irq_no & 7));
  if ((irq_no <= 7) && !(BX_PIC_THIS s.master_pic.IRQ_in & mask)) {
    BX_DEBUG(("IRQ line %d now high", irq_no));
    BX_PIC_THIS s.master_pic.IRQ_in |= mask;
    BX_PIC_THIS s.master_pic.irr |= mask;
    pic_service(& BX_PIC_THIS s.master_pic);
  } else if ((irq_no > 7) && (irq_no <= 15) &&
             !(BX_PIC_THIS s.slave_pic.IRQ_in & mask)) {
    BX_DEBUG(("IRQ line %d now high", irq_no));
    BX_PIC_THIS s.slave_pic.IRQ_in |= mask;
    BX_PIC_THIS s.slave_pic.irr |= mask;
    pic_service(& BX_PIC_THIS s.slave_pic);
  }
}

void bx_pic_c::set_mode(bool ma_sl, Bit8u mode)
{
  if (ma_sl) {
    BX_PIC_THIS s.master_pic.edge_level = mode;
  } else {
    BX_PIC_THIS s.slave_pic.edge_level = mode;
  }
}

void bx_pic_c::clear_highest_interrupt(bx_pic_t *pic)
{
  int irq;
  int lowest_priority;
  int highest_priority;

  /* clear highest current in service bit */
  lowest_priority = pic->lowest_priority;
  highest_priority = lowest_priority + 1;
  if(highest_priority > 7)
    highest_priority = 0;

  irq = highest_priority;
  do {
    if (pic->isr & (1 << irq)) {
      pic->isr &= ~(1 << irq);
      break; /* Return mask of bit cleared. */
    }

    irq ++;
    if(irq > 7)
      irq = 0;
  } while(irq != highest_priority);
}

void bx_pic_c::pic_service(bx_pic_t *pic)
{
  Bit8u unmasked_requests;
  Bit8u irq, isr, max_irq;
  Bit8u highest_priority = pic->lowest_priority + 1;
  if(highest_priority > 7)
    highest_priority = 0;

  isr = pic->isr;
  if (pic->special_mask) {
    /* all priorities may be enabled.  check all IRR bits except ones
     * which have corresponding ISR bits set
     */
    max_irq = highest_priority;
  } else { /* normal mode */
    /* Find the highest priority IRQ that is enabled due to current ISR */
    max_irq = highest_priority;
    if (isr) {
      while ((isr & (1 << max_irq)) == 0) {
        max_irq++;
        if(max_irq > 7)
          max_irq = 0;
      }
      if (max_irq == highest_priority) return; /* Highest priority interrupt in-service,
                                                * no other priorities allowed */
      if (max_irq > 7) BX_PANIC(("error in pic_service()"));
    }
  }

  /* now, see if there are any higher priority requests */
  if ((unmasked_requests = (pic->irr & ~pic->imr))) {
    irq = highest_priority;
    do {
      /* for special mode, since we're looking at all IRQ's, skip if
       * current IRQ is already in-service
       */
      if (!(pic->special_mask && ((isr >> irq) & 0x01))) {
        if (!pic->INT && (unmasked_requests & (1 << irq))) {
          BX_DEBUG(("signalling IRQ #%u", pic->master ? irq : irq + 8));
          pic->INT = 1;
          pic->irq = irq;
          if (pic->master) {
            BX_RAISE_INTR();
          } else {
            BX_PIC_THIS raise_irq(2); /* request IRQ 2 on master pic */
          }
          return;
        } /* if (unmasked_requests & ... */
      }
      irq++;
      if(irq > 7)
        irq = 0;
    } while (irq != max_irq); /* do ... */
  } else if (pic->INT) {
    /* deassert INT if request is masked now */
    if (pic->master) {
      BX_CLEAR_INTR();
    } else {
      BX_PIC_THIS lower_irq(2);
    }
    pic->INT = 0;
  }
}

/* CPU handshakes with PIC after acknowledging interrupt */
Bit8u bx_pic_c::IAC(void)
{
  Bit8u vector;
  Bit8u irq;

  BX_CLEAR_INTR();
  BX_PIC_THIS s.master_pic.INT = 0;
  // Check for spurious interrupt
  if ((BX_PIC_THIS s.master_pic.irr & ~BX_PIC_THIS s.master_pic.imr) == 0) {
    return (BX_PIC_THIS s.master_pic.interrupt_offset + 7);
  }
  // In level sensitive mode don't clear the irr bit.
  if (!(BX_PIC_THIS s.master_pic.edge_level & (1 << BX_PIC_THIS s.master_pic.irq)))
    BX_PIC_THIS s.master_pic.irr &= ~(1 << BX_PIC_THIS s.master_pic.irq);
  // In autoeoi mode don't set the isr bit.
  if (!BX_PIC_THIS s.master_pic.auto_eoi)
    BX_PIC_THIS s.master_pic.isr |= (1 << BX_PIC_THIS s.master_pic.irq);
  else if (BX_PIC_THIS s.master_pic.rotate_on_autoeoi)
    BX_PIC_THIS s.master_pic.lowest_priority = BX_PIC_THIS s.master_pic.irq;

  if (BX_PIC_THIS s.master_pic.irq != 2) {
    irq    = BX_PIC_THIS s.master_pic.irq;
    vector = irq + BX_PIC_THIS s.master_pic.interrupt_offset;
  } else { /* IRQ2 = slave pic IRQ8..15 */
    BX_PIC_THIS s.slave_pic.INT = 0;
    BX_PIC_THIS s.master_pic.IRQ_in &= ~(1 << 2);
    // Check for spurious interrupt
    if ((BX_PIC_THIS s.slave_pic.irr & ~BX_PIC_THIS s.slave_pic.imr) == 0) {
      return (BX_PIC_THIS s.slave_pic.interrupt_offset + 7);
    }
    irq    = BX_PIC_THIS s.slave_pic.irq;
    vector = irq + BX_PIC_THIS s.slave_pic.interrupt_offset;
    // In level sensitive mode don't clear the irr bit.
    if (!(BX_PIC_THIS s.slave_pic.edge_level & (1 << BX_PIC_THIS s.slave_pic.irq)))
      BX_PIC_THIS s.slave_pic.irr &= ~(1 << BX_PIC_THIS s.slave_pic.irq);
    // In autoeoi mode don't set the isr bit.
    if (!BX_PIC_THIS s.slave_pic.auto_eoi)
      BX_PIC_THIS s.slave_pic.isr |= (1 << BX_PIC_THIS s.slave_pic.irq);
    else if (BX_PIC_THIS s.slave_pic.rotate_on_autoeoi)
      BX_PIC_THIS s.slave_pic.lowest_priority = BX_PIC_THIS s.slave_pic.irq;
    pic_service(& BX_PIC_THIS s.slave_pic);
    irq += 8; // for debug printing purposes
  }

  pic_service(& BX_PIC_THIS s.master_pic);

  BX_DBG_IAC_REPORT(vector, irq);
  return(vector);
}

#if BX_DEBUGGER
void bx_pic_c::debug_dump(int argc, char **argv)
{
  dbg_printf("i8259A PIC\n\n");
  dbg_printf("master IMR = %02x\n", BX_PIC_THIS s.master_pic.imr);
  dbg_printf("master ISR = %02x\n", BX_PIC_THIS s.master_pic.isr);
  dbg_printf("master IRR = %02x\n", BX_PIC_THIS s.master_pic.irr);
  dbg_printf("master IRQ = %02x\n", BX_PIC_THIS s.master_pic.irq);
  dbg_printf("slave IMR = %02x\n", BX_PIC_THIS s.slave_pic.imr);
  dbg_printf("slave ISR = %02x\n", BX_PIC_THIS s.slave_pic.isr);
  dbg_printf("slave IRR = %02x\n", BX_PIC_THIS s.slave_pic.irr);
  dbg_printf("slave IRQ = %02x\n", BX_PIC_THIS s.slave_pic.irq);
  if (argc > 0) {
    dbg_printf("\nAdditional options not supported\n");
  }
}
#endif
