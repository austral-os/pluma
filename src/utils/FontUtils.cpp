#include "utils/FontUtils.hpp"

#include <algorithm>
#include <fontconfig/fontconfig.h>
#include <set>

namespace pluma_app {
namespace utils {

std::vector<std::string> FontUtils::get_installed_fonts() {
  std::set<std::string> unique_fonts;

  FcInit();

  FcPattern *pat = FcPatternCreate();
  FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, (char *)0);
  FcFontSet *fs = FcFontList(0, pat, os);

  if (fs) {
    for (int i = 0; i < fs->nfont; i++) {
      FcChar8 *family;
      if (FcPatternGetString(fs->fonts[i], FC_FAMILY, 0, &family) == FcResultMatch) {
        unique_fonts.insert(reinterpret_cast<char *>(family));
      }
    }
    FcFontSetDestroy(fs);
  }

  FcObjectSetDestroy(os);
  FcPatternDestroy(pat);

  // FcFini(); // Usually not necessary and can cause issues if called multiple times

  std::vector<std::string> result(unique_fonts.begin(), unique_fonts.end());
  return result;
}

} // namespace utils
} // namespace pluma_app
