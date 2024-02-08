#ifndef CONFIG_HH
#define CONFIG_HH


typedef enum {
    HS_MIN = 1,
    HS_LAZY,
    HS_GREEDY,
    HS_MAX
} HsOption;


///contains all global variables (mainly solver's command-line options)
class HsConfig {
protected:
    virtual ~HsConfig() = 0; // Trick to avoid any instantiation 
public:
    static HsOption hsOption;
};

#endif
