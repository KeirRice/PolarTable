#ifndef __SIMPLE_RPC_H__
#define __SIMPLE_RPC_H__

#include <Stream.h>

#ifndef SIMPLE_RPC_PORT
#define SIMPLE_RPC_PORT Serial
#endif // SIMPLE_RPC_PORT

#include "interface.tcc"

#define _VERSION "2.0.1"


const char *_version(void) {
  return _VERSION;
}

byte _ping(byte data) {
  return data;
}

class SimpleRPC_{

	public:
		SimpleRPC_() : _stream(&Serial) {};
		SimpleRPC_(Stream &s) : _stream(&s) {}; 

		void begin(Stream &s){
			_stream = &s;
		};

		void end(){};

		/**
		 * RPC interface.
		 *
		 * This function expects parameter pairs (function pointer, documentation).
		 *
		 * @arg {Args...} args - Parameter pairs (function pointer, documentation).
		 */
		template<class... Args>
		void interface(Args... args) {
			// TODO: Check for null pntr?
		  rpcInterface(
		  	(*_stream),
		    _version, "version: Protocol version. @return: Version string.",
		    _ping, "ping: Echo a value. @data: Value. @return: Value of data.",
		    args...);
		}

	private:
		Stream* _stream = nullptr;

};

typedef SimpleRPC_ SimpleRPC;

#endif
