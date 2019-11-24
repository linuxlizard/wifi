#ifndef NLNAMES_H
#define NLNAMES_H

#ifdef __cplusplus
extern "C" {
#endif

char* to_string_nl80211_commands(enum nl80211_commands val);
const char * to_string_nl80211_attrs(enum nl80211_attrs val);

#ifdef __cplusplus
}  // end extern "C"
#endif

#endif

