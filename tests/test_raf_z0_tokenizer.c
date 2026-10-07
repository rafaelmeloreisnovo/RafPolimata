#include "../tools/raf_z0_tokenizer.h"

#define RAF_Z0_EXPECT(condition) \
    do {                         \
        if (!(condition)) {      \
            return __LINE__;     \
        }                        \
    } while (0)

int main(void)
{
    const unsigned char blank[] = " ";
    const unsigned char tabs_and_newline[] = "\t\n\r ";
    const unsigned char underscores[] = "____";
    const unsigned char visible[] = "z0";
    const unsigned char mixed[] = " __z0";
    raf_z0_observation observation;

    observation = raf_z0_observe_bytes(0, 0UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_ABSOLUTE_EMPTY);
    RAF_Z0_EXPECT(observation.byte_count == 0UL);
    RAF_Z0_EXPECT(observation.blank_count == 0UL);
    RAF_Z0_EXPECT(observation.underscore_count == 0UL);
    RAF_Z0_EXPECT(observation.visible_count == 0UL);

    observation = raf_z0_observe_bytes(0, 1UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_MISSING_SOURCE);
    RAF_Z0_EXPECT(observation.byte_count == 1UL);

    observation = raf_z0_observe_bytes(blank, 1UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_BLANK_FIELD);
    RAF_Z0_EXPECT(observation.blank_count == 1UL);
    RAF_Z0_EXPECT(observation.underscore_count == 0UL);
    RAF_Z0_EXPECT(observation.visible_count == 0UL);

    observation = raf_z0_observe_bytes(tabs_and_newline, 4UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_BLANK_FIELD);
    RAF_Z0_EXPECT(observation.blank_count == 4UL);

    observation = raf_z0_observe_bytes(underscores, 4UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_UNDERSCORE_FIELD);
    RAF_Z0_EXPECT(observation.underscore_count == 4UL);
    RAF_Z0_EXPECT(observation.blank_count == 0UL);
    RAF_Z0_EXPECT(observation.visible_count == 0UL);

    observation = raf_z0_observe_bytes(visible, 2UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_VISIBLE_FIELD);
    RAF_Z0_EXPECT(observation.visible_count == 2UL);

    observation = raf_z0_observe_bytes(mixed, 5UL);
    RAF_Z0_EXPECT(observation.kind == RAF_Z0_MIXED_FIELD);
    RAF_Z0_EXPECT(observation.blank_count == 1UL);
    RAF_Z0_EXPECT(observation.underscore_count == 2UL);
    RAF_Z0_EXPECT(observation.visible_count == 2UL);

    return 0;
}
