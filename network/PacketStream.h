//Util class to make writing/reading packets from
//a byte array easier
class PacketStream
    {
  uint8_t* _start;
  uint8_t* _readPointer;
  uint8_t* _writePointer;

    public:
      PacketStream(uint8_t* buffer) : _start(buffer), _readPointer(buffer) {
        //reserve space for packet size, sender and receiver
        _writePointer = buffer + 4 + 6 + 6;
      }

      //Retrns the start of the stream
      uint8_t* start() const
      {
        return _start;
      }

      //Reads the packet size from the stream
      uint32_t readSize()
      {
        uint32_t size;
        *this >> size;
        return size;
      }

      //Reads the sender or receiver mac address
      ns3::Mac48Address readMac()
      {
        uint8_t buf[6];
        for(int i = 0; i < 6; i++) {
          *this >> buf[i];
        }
        ns3::Mac48Address addr;
        addr.CopyFrom(buf);
        return addr;
      }

      //Writes the sener or receiver mac address
      void writeMac(ns3::Address addr)
      {
        uint8_t buf[6];
        addr.CopyTo(buf);
        for(int i = 0; i < 6; i++) {
          *this << buf[i];
        }
      }

      //Finishes the packet stream and writes the packet size
      //in the header
      uint32_t finish(ns3::Address sender, ns3::Address receiver)
      {
        uint8_t* end = _writePointer;
        uint32_t size = end - _start; //calculate packet size

        _writePointer = _start;

        *this << size;

        writeMac(sender);

        writeMac(receiver);

        _writePointer = end;

        return size;
      }

      //Writes the specified type into the underlying
      //byte array and advances the read/write position
      template<typename T>
      PacketStream& operator<<(T value)
      {
        *reinterpret_cast<T*>(_writePointer) = value;
        _writePointer += sizeof(T);
        return *this;
      }

      //Reads a value into the specified type from the
      //underlying byte array and advances the read/write
      //position
      template<typename T>
      PacketStream& operator>>(T& value)
      {
        value = *reinterpret_cast<T*>(_readPointer);
        _readPointer += sizeof(T);
        return *this;
      }
    };
