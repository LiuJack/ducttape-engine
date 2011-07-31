
// ----------------------------------------------------------------------------
// This file is part of the Ducttape Project (http://ducttape-dev.org) and is
// licensed under the GNU LESSER PUBLIC LICENSE version 3. For the full license
// text, please see the LICENSE file in the root of this project or at
// http://www.gnu.org/licenses/lgpl.html
// ----------------------------------------------------------------------------

#include <Utils/EnumHelper.hpp>

#include <SFML/Network.hpp>

#include <cstdint>

namespace dt {

EnumHelper::EnumHelper(void* e)
    : mEnumObj(e) {}

uint32_t EnumHelper::Get() {
    return (uint32_t)(*((uint32_t*)mEnumObj));
}

void EnumHelper::Set(uint32_t i) {
   *((uint32_t*)mEnumObj) = i;
}

sf::Packet& operator << (sf::Packet& p, EnumHelper e) {
    uint32_t i = e.Get();
    p << i;
    return p;
}

sf::Packet& operator >> (sf::Packet& p, EnumHelper e) {
    uint32_t i;
    p >> i;
    e.Set(i);
    return p;
}

} // namespace dt
