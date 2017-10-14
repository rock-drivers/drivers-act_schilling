#ifndef ACT_SCHILLING_PANTILTTYPES_HPP
#define ACT_SCHILLING_PANTILTTYPES_HPP

#include <base/Time.hpp>
#include <vector>

namespace act_schilling {


  /** This structure holds JoyStickConfig data **/
  struct JoyStickMapping {
    //! timestamp
    base::Time  time;

    uint8_t triggerButton;
    std::vector<int> defaultValButtons;

    uint8_t configureButton;
    uint8_t inputAxisNumber;
    uint8_t inputAxisDimension;
    bool inputAxisInvert;

    JoyStickMapping() :
      time(base::Time::now()), configureButton(0), inputAxisNumber(0), inputAxisDimension(0), inputAxisInvert(true)
    {}
  };

  /** This structure holds PanTiltDefaultPos data */
  struct PanTiltDefaultPos {
    //! timestamp
    base::Time  time;
    std::vector<int> pos_value;

    PanTiltDefaultPos() :
      time(base::Time::now()), pos_value(0)
    {}
    PanTiltDefaultPos(std::vector<int> i_pos_value) :
      time(base::Time::now()), pos_value(i_pos_value)
    {}
  };
}


#endif
