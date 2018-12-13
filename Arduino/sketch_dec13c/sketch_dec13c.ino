  typedef volatile const uint8_t reg_r_t; /**< Read only 8-bit register (volatile const unsigned int) */
  typedef volatile uint8_t reg_rw_t;    /**< Read-Write 8-bit register (volatile unsigned int) */
  
  typedef volatile reg_rw_t *port_ptr_t;
  
  typedef reg_rw_t port_t;
  typedef reg_rw_t reg_ddr_t;  // Data Direction Register
  typedef reg_rw_t reg_port_t;  // Data Register
  typedef reg_r_t reg_pin_t;  // Input Pins Register
  
  class PortID {

    const uint8_t mPortNumber;

    reg_ddr_t *mDDRPort;
    reg_port_t *mPort;
    const reg_pin_t *mPin;

  public:
        constexpr PortID(const uint8_t port_number, auto ddr, auto port, auto pin) : 
    mPortNumber(port_number),
    mDDRPort(ddr),
    mPort(port),
    mPin(pin)
    {};
  };

  void setup(){
    // reg_ddr_t *mDDRPort;
    // reg_port_t *mPort;
    // const reg_pin_t *mPin;

    constexpr static const PortID PortA(1, &DDRA, &DDRA, &DDRA);
    };
  void loop(){};
