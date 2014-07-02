#include <boost/preprocessor/repetition/repeat.hpp>
#define GET_STR_AUX(_, i, str) (sizeof(str) > (i) ? str[(i)] : 0),
#define GET_STR(str) BOOST_PP_REPEAT(64,GET_STR_AUX,str) 0
