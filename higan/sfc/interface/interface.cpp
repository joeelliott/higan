#include <sfc/sfc.hpp>

namespace SuperFamicom {

#define returns(T) T { return ([&] { struct With : T { With() {
#define $ }}; return With(); })(); }

Settings settings;

auto Interface::information() -> returns(Information) {
  manufacturer = "Nintendo";
  name         = "Super Famicom";
  extension    = "sfc";
  resettable   = true;
}$

auto Interface::manifest() -> string {
  return cartridge.manifest();
}

auto Interface::title() -> string {
  return cartridge.title();
}

auto Interface::display() -> returns(Display) {
  type   = Display::Type::CRT;
  colors = 1 << 19;
  width  = 256;
  height = 240;
  internalWidth  = 512;
  internalHeight = 480;
  aspectCorrection = 8.0 / 7.0;
  if(Region::NTSC()) refreshRate = system.cpuFrequency() / (262.0 * 1364.0);
  if(Region::PAL())  refreshRate = system.cpuFrequency() / (312.0 * 1364.0);
}$

auto Interface::color(uint32 color) -> uint64 {
  uint r = color.bits( 0, 4);
  uint g = color.bits( 5, 9);
  uint b = color.bits(10,14);
  uint l = color.bits(15,18);

  //luma=0 is not 100% black; but it's much darker than normal linear scaling
  //exact effect seems to be analog; requires > 24-bit color depth to represent accurately
  double L = (1.0 + l) / 16.0 * (l ? 1.0 : 0.25);
  uint64 R = L * image::normalize(r, 5, 16);
  uint64 G = L * image::normalize(g, 5, 16);
  uint64 B = L * image::normalize(b, 5, 16);

  if(settings.colorEmulation) {
    static const uint8 gammaRamp[32] = {
      0x00, 0x01, 0x03, 0x06, 0x0a, 0x0f, 0x15, 0x1c,
      0x24, 0x2d, 0x37, 0x42, 0x4e, 0x5b, 0x69, 0x78,
      0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0,
      0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0xff,
    };
    R = L * gammaRamp[r] * 0x0101;
    G = L * gammaRamp[g] * 0x0101;
    B = L * gammaRamp[b] * 0x0101;
  }

  return R << 32 | G << 16 | B << 0;
}

auto Interface::loaded() -> bool {
  return system.loaded();
}

auto Interface::sha256() -> string {
  return cartridge.sha256();
}

auto Interface::load() -> bool {
  return system.load(this);
}

auto Interface::save() -> void {
  system.save();
}

auto Interface::unload() -> void {
  save();
  system.unload();
}

auto Interface::ports() -> vector<Port> { return {
  {ID::Port::Controller1, "Controller Port 1"},
  {ID::Port::Controller2, "Controller Port 2"},
  {ID::Port::Expansion,   "Expansion Port"   }};
}

auto Interface::devices(uint port) -> vector<Device> {
  if(port == ID::Port::Controller1) return {
    {ID::Device::None,    "None"   },
    {ID::Device::Gamepad, "Gamepad"},
    {ID::Device::Mouse,   "Mouse"  }
  };

  if(port == ID::Port::Controller2) return {
    {ID::Device::None,          "None"          },
    {ID::Device::Gamepad,       "Gamepad"       },
    {ID::Device::Mouse,         "Mouse"         },
    {ID::Device::SuperMultitap, "Super Multitap"},
    {ID::Device::SuperScope,    "Super Scope"   },
    {ID::Device::Justifier,     "Justifier"     },
    {ID::Device::Justifiers,    "Justifiers"    }
  };

  if(port == ID::Port::Expansion) return {
    {ID::Device::None,        "None"       },
    {ID::Device::Satellaview, "Satellaview"},
    {ID::Device::S21FX,       "21fx"       }
  };

  return {};
}

auto Interface::inputs(uint device) -> vector<Input> {
  using Type = Input::Type;

  if(device == ID::Device::None) return {
  };

  if(device == ID::Device::Gamepad) return {
    {Type::Hat,     "Up"    },
    {Type::Hat,     "Down"  },
    {Type::Hat,     "Left"  },
    {Type::Hat,     "Right" },
    {Type::Button,  "B"     },
    {Type::Button,  "A"     },
    {Type::Button,  "Y"     },
    {Type::Button,  "X"     },
    {Type::Trigger, "L"     },
    {Type::Trigger, "R"     },
    {Type::Control, "Select"},
    {Type::Control, "Start" }
  };

  if(device == ID::Device::Mouse) return {
    {Type::Axis,   "X-axis"},
    {Type::Axis,   "Y-axis"},
    {Type::Button, "Left"  },
    {Type::Button, "Right" }
  };

  if(device == ID::Device::SuperMultitap) {
    vector<Input> inputs;
    for(uint p = 2; p <= 5; p++) inputs.append({
      {Type::Hat,     {"Port ", p, " - ", "Up"    }},
      {Type::Hat,     {"Port ", p, " - ", "Down"  }},
      {Type::Hat,     {"Port ", p, " - ", "Left"  }},
      {Type::Hat,     {"Port ", p, " - ", "Right" }},
      {Type::Button,  {"Port ", p, " - ", "B"     }},
      {Type::Button,  {"Port ", p, " - ", "A"     }},
      {Type::Button,  {"Port ", p, " - ", "Y"     }},
      {Type::Button,  {"Port ", p, " - ", "X"     }},
      {Type::Trigger, {"Port ", p, " - ", "L"     }},
      {Type::Trigger, {"Port ", p, " - ", "R"     }},
      {Type::Control, {"Port ", p, " - ", "Select"}},
      {Type::Control, {"Port ", p, " - ", "Start" }}
    });
    return inputs;
  }

  if(device == ID::Device::SuperScope) return {
    {Type::Axis,    "X-axis" },
    {Type::Axis,    "Y-axis" },
    {Type::Control, "Trigger"},
    {Type::Control, "Cursor" },
    {Type::Control, "Turbo"  },
    {Type::Control, "Pause"  }
  };

  if(device == ID::Device::Justifier) return {
    {Type::Axis,    "X-axis" },
    {Type::Axis,    "Y-axis" },
    {Type::Control, "Trigger"},
    {Type::Control, "Start"  }
  };

  if(device == ID::Device::Justifiers) return {
    {Type::Axis,    "Port 1 - X-axis" },
    {Type::Axis,    "Port 1 - Y-axis" },
    {Type::Control, "Port 1 - Trigger"},
    {Type::Control, "Port 1 - Start"  },
    {Type::Axis,    "Port 2 - X-axis" },
    {Type::Axis,    "Port 2 - Y-axis" },
    {Type::Control, "Port 2 - Trigger"},
    {Type::Control, "Port 2 - Start"  }
  };

  if(device == ID::Device::Satellaview) return {
  };

  if(device == ID::Device::S21FX) return {
  };

  return {};
}

auto Interface::connected(uint port) -> uint {
  if(port == ID::Port::Controller1) return settings.controllerPort1;
  if(port == ID::Port::Controller2) return settings.controllerPort2;
  if(port == ID::Port::Expansion) return settings.expansionPort;
  return 0;
}

auto Interface::connect(uint port, uint device) -> void {
  if(port == ID::Port::Controller1) controllerPort1.connect(settings.controllerPort1 = device);
  if(port == ID::Port::Controller2) controllerPort2.connect(settings.controllerPort2 = device);
  if(port == ID::Port::Expansion) expansionPort.connect(settings.expansionPort = device);
}

auto Interface::power() -> void {
  system.power(/* reset = */ false);
}

auto Interface::reset() -> void {
  system.power(/* reset = */ true);
}

auto Interface::run() -> void {
  system.run();
}

auto Interface::rtc() -> bool {
  if(cartridge.has.EpsonRTC) return true;
  if(cartridge.has.SharpRTC) return true;
  return false;
}

auto Interface::rtcSynchronize() -> void {
  if(cartridge.has.EpsonRTC) epsonrtc.sync();
  if(cartridge.has.SharpRTC) sharprtc.sync();
}

auto Interface::serialize() -> serializer {
  system.runToSave();
  return system.serialize();
}

auto Interface::unserialize(serializer& s) -> bool {
  return system.unserialize(s);
}

auto Interface::cheatSet(const string_vector& list) -> void {
  cheat.reset();
  #if defined(SFC_SUPERGAMEBOY)
  if(cartridge.has.ICD) return GameBoy::cheat.assign(list);
  #endif
  cheat.assign(list);
}

auto Interface::cap(const string& name) -> bool {
  if(name == "Fast PPU") return true;
  if(name == "Fast PPU/No Sprite Limit") return true;
  if(name == "Fast PPU/Hires Mode 7") return true;
  if(name == "Fast DSP") return true;
  if(name == "Mode") return true;
  if(name == "Blur Emulation") return true;
  if(name == "Color Emulation") return true;
  if(name == "Scanline Emulation") return true;
  return false;
}

auto Interface::get(const string& name) -> any {
  if(name == "Fast PPU") return settings.fastPPU;
  if(name == "Fast PPU/No Sprite Limit") return settings.fastPPUNoSpriteLimit;
  if(name == "Fast PPU/Hires Mode 7") return settings.fastPPUHiresMode7;
  if(name == "Fast DSP") return settings.fastDSP;
  if(name == "Blur Emulation") return settings.blurEmulation;
  if(name == "Color Emulation") return settings.colorEmulation;
  if(name == "Scanline Emulation") return settings.scanlineEmulation;
  return {};
}

auto Interface::set(const string& name, const any& value) -> bool {
  if(name == "Fast PPU" && value.is<bool>()) {
    settings.fastPPU = value.get<bool>();
    return true;
  }
  if(name == "Fast PPU/No Sprite Limit" && value.is<bool>()) {
    settings.fastPPUNoSpriteLimit = value.get<bool>();
    return true;
  }
  if(name == "Fast PPU/Hires Mode 7" && value.is<bool>()) {
    settings.fastPPUHiresMode7 = value.get<bool>();
    return true;
  }
  if(name == "Fast DSP" && value.is<bool>()) {
    settings.fastDSP = value.get<bool>();
    return true;
  }
  if(name == "Blur Emulation" && value.is<bool>()) {
    settings.blurEmulation = value.get<bool>();
    return true;
  }
  if(name == "Color Emulation" && value.is<bool>()) {
    settings.colorEmulation = value.get<bool>();
    Emulator::video.setPalette();
    return true;
  }
  if(name == "Scanline Emulation" && value.is<bool>()) {
    settings.scanlineEmulation = value.get<bool>();
    return true;
  }
  return false;
}

#undef returns
#undef $

}
