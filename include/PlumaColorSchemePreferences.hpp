#pragma once

#include <algorithm>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace pluma_app::color_scheme_preferences {

inline constexpr const char *kSectionName = "general";
inline constexpr const char *kVariantKey = "variant";
inline constexpr const char *kDefaultVariant = "default";

inline bool variant_exists(const std::vector<std::string> &variants, const std::string &variant) {
    if (variant == kDefaultVariant) {
        return true;
    }
    return std::find(variants.begin(), variants.end(), variant) != variants.end();
}

inline std::string normalize_variant(const std::string &variant,
                                     const std::vector<std::string> &available_variants) {
    return variant_exists(available_variants, variant) ? variant : kDefaultVariant;
}

inline std::string variant_from_json(const nlohmann::json &j,
                                     const std::vector<std::string> &available_variants) {
    if (j.is_null() || !j.is_object()) {
        return kDefaultVariant;
    }

    const auto configured = j.value(kVariantKey, std::string(kDefaultVariant));
    return normalize_variant(configured, available_variants);
}

inline nlohmann::json variant_to_json(const std::string &variant,
                                      const std::vector<std::string> &available_variants) {
    return {{kVariantKey, normalize_variant(variant, available_variants)}};
}

} // namespace pluma_app::color_scheme_preferences
