#include <czmq.h>
#include <string>

#include "asset_msg.h"
#include "asset_types.h"

class asset_manager {
    public:
        zmsg_t* get_item(std::string type, std::string id);
        zmsg_t* get_items(std::string type);
        static byte type_to_byte(std::string type);
        static std::string byte_to_type(byte type);
};
