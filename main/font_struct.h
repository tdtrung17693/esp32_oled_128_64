typedef struct {
    uint8_t first_char;
} FontInfo;

typedef struct {
    uint32_t offset;
    uint8_t width;
    uint8_t height;
    uint8_t advance_width;
    uint8_t advance_height;
    uint8_t descent;
    uint8_t ascent;
} FontDesc;