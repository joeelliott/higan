//Mega CD

struct MCD : M68K, Thread {
  Memory::Readable<uint16> bios;  //BIOS ROM
  Memory::Writable<uint16> pram;  //program RAM
  Memory::Writable<uint16> wram;  //work RAM
  Memory::Writable< uint8> bram;  //backup RAM

  //mcd.cpp
  auto load() -> void;
  auto unload() -> void;

  auto main() -> void;
  auto step(uint clocks) -> void override;
  auto power(bool reset) -> void;

  //bus.cpp
  auto read(uint1 upper, uint1 lower, uint24 address, uint16 data = 0) -> uint16 override;
  auto write(uint1 upper, uint1 lower, uint24 address, uint16 data) -> void override;

  //bus-external.cpp
  auto external_read(uint1 upper, uint1 lower, uint22 address, uint16 data) -> uint16;
  auto external_write(uint1 upper, uint1 lower, uint22 address, uint16 data) -> void;

  //io.cpp
  auto readIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> uint16;
  auto writeIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> void;

  //io-external.cpp
  auto external_readIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> uint16;
  auto external_writeIO(uint1 upper, uint1 lower, uint24 address, uint16 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  struct IO {
    uint1 run;
    uint1 request;
    uint1 halt = 1;

    uint1 wramMode;     //MODE: 0 = 2mbit mode, 1 = 1mbit mode
    uint1 wramSwitch;
    uint1 wramSelect;
    uint2 wramPriority;
    uint2 pramBank;
    uint8 pramProtect;

    uint16 counter;
    uint16 decoder;
  } io;

  struct LED {
    uint1 red;
    uint1 green;
  } led;

  struct IRQ {
    //irq.cpp
    auto raise() -> bool;
    auto lower() -> bool;
    static auto synchronize() -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    uint1 enable;
    uint1 pending;
  };

  struct IRQs {
    uint1 pending;

    IRQ reset;
    IRQ subcode;
  } irq;

  struct External {
    IRQ irq;
  } external;

  struct Communcation {
     uint8 cfm;
     uint8 cfs;
    uint16 command[8];
    uint16 status [8];
  } communication;

  struct CDC {
    //cdc.cpp
    auto poll() -> void;
    auto clock() -> void;
    auto read() -> uint8;
    auto write(uint8 data) -> void;
    auto power(bool reset) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

     uint1 dsr;
     uint1 edt;
     uint4 address;
     uint3 destination;
    uint12 stopwatch;

    struct IRQ : MCD::IRQ {
      MCD::IRQ decoder;   //DECEIN + DECI
      MCD::IRQ transfer;  //DTEIEN + DTEI
      MCD::IRQ command;   //CMDIEN + CMDI
    } irq;

    struct Command {
      uint8 fifo[8];  //COMIN
      uint3 read;
      uint3 write;
      uint1 empty = 1;
    } command;

    struct Status {
      uint8 fifo[8];    //SBOUT
      uint3 read;
      uint3 write;
      uint1 empty = 1;
      uint1 enable;     //SOUTEN
      uint1 active;     //STEN
      uint1 busy;       //STBSY
      uint1 wait;       //STWAI
    } status;

    struct Tranfer {
      uint16 source;
      uint16 target;
      uint16 pointer;
      uint12 length;

       uint1 enable;  //DOUTEN
       uint1 active;  //DTEN
       uint1 busy;    //DTBSY
       uint1 wait;    //DTWAI
    } transfer;

    enum : uint { Mode1 = 0, Mode2 = 1 };
    enum : uint { Form1 = 0, Form2 = 1 };
    struct Decoder {
      uint1 enable;  //DECEN
      uint1 mode;    //MODE
      uint1 form;    //FORM
    } decoder;

    struct Header {
      uint8 minutes;
      uint8 seconds;
      uint8 blocks;
      uint8 mode = 1;
    } header;

    struct Subheader {
      uint8 file;
      uint8 channel;
      uint8 submode;
      uint8 coding;
    } subheader;

    struct Control {
      uint1 head;               //SHDREN: 0 = read header, 1 = read subheader
      uint1 mode;               //MODE
      uint1 form;               //FORM
      uint1 commandBreak;       //CMDBK
      uint1 modeByteCheck;      //MBCKRQ
      uint1 erasureRequest;     //ERAMRQ
      uint1 writeRequest;       //WRRQ
      uint1 pCodeCorrection;    //PRQ
      uint1 qCodeCorrection;    //QRQ
      uint1 autoCorrection;     //AUTOQ
      uint1 errorCorrection;    //E01RQ
      uint1 edcCorrection;      //EDCRQ
      uint1 correctionWrite;    //COWREN
      uint1 descramble;         //DSCREN
      uint1 syncDetection;      //SYDEN
      uint1 syncInterrupt;      //SYIEN
      uint1 erasureCorrection;  //ERAMSL
      uint1 statusTrigger;      //STENTRG
      uint1 statusControl;      //STENCTL
    } control;
  } cdc;

  struct CDD {
    struct DriveStatus { enum : uint {
      Stop,             //disc spinning stopped
      Play,             //playing data or music
      Seek,             //move to specified time
      Scan,             //searching with a scan command
      Pause,            //paused at a specific time
      Open,             //tray is open
      ChecksumError,    //invalid communication checksum
      CommandError,     //missing command
      FunctionError,    //unknown error
      TableOfContents,  //reading TOC
      TrackMove,        //accessing in high-speed mode
      NoDisc,           //no disc in tray or cannot focus
      DiscEnd,          //pickup is in the lead-out area
      Tray,             //door is moving via open/close commands
      Test,             //in test mode
    };};

    //cdd.cpp
    auto clock() -> void;
    auto process() -> void;
    auto valid() -> bool;
    auto checksum() -> void;
    auto power(bool reset) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    IRQ irq;
    uint16 counter;

    struct Fader {
       uint1 spindleSpeed;  //0 = normal-speed, 1 = double-speed
       uint2 deemphasis;    //0 = off, 1 = 44.1khz, 2 = 32khz, 3 = 48khz
      uint11 volume;
    } fader;

    struct IO {
      uint4 status = DriveStatus::Stop;
    } io;

    uint1 hostClockEnable;

    struct Status {
      inline auto operator[](uint index) -> uint4& { return data[index]; }

      uint1 receiving;
      uint4 data[10];
    } status;

    struct Command {
      inline auto operator[](uint index) -> uint4& { return data[index]; }

      uint1 sending;
      uint4 data[10];
    } command;
  } cdd;

  struct Timer {
    //timer.cpp
    auto clock() -> void;
    auto power(bool reset) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    IRQ irq;
    uint8 counter;
  } timer;

  struct GPU {
    //gpu.cpp
    auto step(uint clocks) -> void;
    auto render() -> void;
    auto start() -> void;
    auto power(bool reset) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    IRQ irq;

    struct Font {
      struct Color {
        uint4 background;
        uint4 foreground;
      } color;
      uint16 data;
    } font;

    struct Stamp {
      uint1 repeat;
      struct Tile {
        uint1 size;  //0 = 16x16, 1 = 32x32
      } tile;
      struct Map {
         uint1 size;  //0 = 256x256, 1 = 4096x4096
        uint17 base;
      } map;
    } stamp;

    struct Image {
      uint17 base;
       uint6 offset;
       uint5 vcells;
       uint8 vdots;
       uint9 hdots;
    } image;

    struct Vector {
      uint17 base;
    } vector;

     uint1 active;
    uint32 counter;
    uint32 period;
  } gpu;

  struct PCM {
    shared_pointer<Stream> stream;
    Memory::Writable<uint8> ram;

    //pcm.cpp
    auto clock() -> void;
    auto read(uint13 address, uint8 data) -> uint8;
    auto write(uint13 address, uint8 data) -> void;
    auto power(bool reset) -> void;

    //serialization.cpp
    auto serialize(serializer&) -> void;

    struct IO {
      uint1 enable;
      uint4 bank;
      uint3 channel;
    } io;

    struct Channel {
       uint1 enable;
       uint8 envelope;
       uint8 pan = 0xff;
      uint16 step;
      uint16 loop;
       uint8 start;
      uint27 address;
    } channels[8];
  } pcm;
};

extern MCD mcd;