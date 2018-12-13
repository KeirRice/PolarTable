  typedef volatile const uint8_t reg_r_t; /**< Read only 8-bit register (volatile const unsigned int) */
  typedef volatile uint8_t reg_rw_t;    /**< Read-Write 8-bit register (volatile unsigned int) */
  
  typedef volatile reg_rw_t *port_ptr_t;
  
  typedef reg_rw_t port_t;
  typedef reg_rw_t reg_ddr_t;  // Data Direction Register
  typedef reg_rw_t reg_port_t;  // Data Register
  typedef reg_r_t reg_pin_t;  // Input Pins Register


  template <class T> class PortPTR{
    uint8_t addr; 
  public:
    constexpr PortPTR(uint8_t i=0) : addr{i} {}
    operator T*() const { return reinterpret_cast<T*>(addr); }
    T* operator->() const { return operator T*(); }
  
    uint8_t* raw() const { return reinterpret_cast<uint8_t*>(addr); }
  };

  constexpr const PortPTR<reg_rw_t> PROGMEM const_port_to_mode_PGM[] = {
    PortPTR<reg_rw_t>(),
    PortPTR<reg_rw_t>(DDRA),
    PortPTR<reg_rw_t>(DDRB),
    PortPTR<reg_rw_t>(DDRC),
    PortPTR<reg_rw_t>(DDRD),
    PortPTR<reg_rw_t>(DDRE),
    PortPTR<reg_rw_t>(DDRF),
    PortPTR<reg_rw_t>(DDRG),
    PortPTR<reg_rw_t>(DDRH),
    PortPTR<reg_rw_t>(),
    PortPTR<reg_rw_t>(DDRJ),
    PortPTR<reg_rw_t>(DDRK),
    PortPTR<reg_rw_t>(DDRL),
  };

  constexpr const PortPTR<reg_rw_t> PROGMEM const_port_to_output_PGM[] = {
    PortPTR<reg_rw_t>(),
    PortPTR<reg_rw_t>(DDRA),
    PortPTR<reg_rw_t>(DDRB),
    PortPTR<reg_rw_t>(DDRC),
    PortPTR<reg_rw_t>(DDRD),
    PortPTR<reg_rw_t>(DDRE),
    PortPTR<reg_rw_t>(DDRF),
    PortPTR<reg_rw_t>(DDRG),
    PortPTR<reg_rw_t>(DDRH),
    PortPTR<reg_rw_t>(),
    PortPTR<reg_rw_t>(DDRJ),
    PortPTR<reg_rw_t>(DDRK),
    PortPTR<reg_rw_t>(DDRL),
  };

  constexpr const PortPTR<reg_r_t> PROGMEM const_port_to_input_PGM[] = {
    PortPTR<reg_r_t>(),
    PortPTR<reg_r_t>(DDRA),
    PortPTR<reg_r_t>(DDRB),
    PortPTR<reg_r_t>(DDRC),
    PortPTR<reg_r_t>(DDRD),
    PortPTR<reg_r_t>(DDRE),
    PortPTR<reg_r_t>(DDRF),
    PortPTR<reg_r_t>(DDRG),
    PortPTR<reg_r_t>(DDRH),
    PortPTR<reg_r_t>(),
    PortPTR<reg_r_t>(DDRJ),
    PortPTR<reg_r_t>(DDRK),
    PortPTR<reg_r_t>(DDRL),
  };

  #define UnpackPort(port) port.raw()

  class PortID {

    const uint8_t mPortNumber;

    reg_ddr_t *mDDRPort;
    reg_port_t *mPort;
    const reg_pin_t *mPin;

  public:

    constexpr PortID(const int port) : 
    mPortNumber(port),
    mDDRPort(const_port_to_mode_PGM[(int)port].raw()),
    mPort(const_port_to_output_PGM[(int)port].raw()),
    mPin(const_port_to_input_PGM[(int)port].raw())
    {};
//    constexpr PortID(const PortID &obj) :
//    mPortNumber(obj.mPortNumber),
//    mDDRPort(obj.mDDRPort),
//    mPort(obj.mPort),
//    mPin(obj.mPin) {}

//    constexpr uint8_t port() const {
//      return mPortNumber;
//    }
//
//    constexpr operator uint8_t() const {
//      return mPortNumber;
//    }

//    inline uint8_t digitalRead() const {
//      return (*mPin);
//    }
//    inline uint8_t digitalRead(uint8_t mask, uint8_t shift=0) const {
//      // Read the masked bits. Use shift value to align the results to LSB end.
//      return ((*mPin) & mask) >> shift;
//    }
//    inline void digitalWrite(uint8_t value) const {
//      (*mPort) = value;
//    }
//    inline void digitalWrite(uint8_t value, uint8_t mask, uint8_t shift=0) const {
//      // Write the value into the masked bits.
//      // Use shift to align value LSB with the start of the mask area.
//      (*mPort) = ((*mPort) & ~mask) | ((value << shift) & mask);
//    }
//
//    void EnablePCInterupt(uint8_t portMask) const;
//    void DisablePCInterupt(uint8_t portMask) const;
  };
  //typedef PortID PortID;

//  constexpr static const PortID PORT_A(1);
//  constexpr static const PortID PORT_B(2);
//  constexpr static const PortID PORT_C(3);

  void setup(){
    reg_ddr_t *mDDRPort;
    // reg_port_t *mPort;
    // const reg_pin_t *mPin;

    const int port = 3;
    mDDRPort = UnpackPort(const_port_to_mode_PGM[port]);

    constexpr static const PortID PORT_A(port);
    };
  void loop(){};
