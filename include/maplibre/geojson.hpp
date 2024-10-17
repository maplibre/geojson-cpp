#pragma once

#include <mapbox/variant.hpp>
#include <maplibre/feature.hpp>
#include <maplibre/geometry.hpp>

namespace maplibre {
namespace geojson {

using empty               = maplibre::geometry::empty;
using point               = maplibre::geometry::point<double>;
using multi_point         = maplibre::geometry::multi_point<double>;
using line_string         = maplibre::geometry::line_string<double>;
using linear_ring         = maplibre::geometry::linear_ring<double>;
using multi_line_string   = maplibre::geometry::multi_line_string<double>;
using polygon             = maplibre::geometry::polygon<double>;
using multi_polygon       = maplibre::geometry::multi_polygon<double>;
using geometry            = maplibre::geometry::geometry<double>;
using geometry_collection = maplibre::geometry::geometry_collection<double>;

using value              = maplibre::feature::value;
using null_value_t       = maplibre::feature::null_value_t;
using identifier         = maplibre::feature::identifier;
using feature            = maplibre::feature::feature<double>;
using feature_collection = maplibre::feature::feature_collection<double>;

using geojson = mapbox::util::variant<geometry, feature, feature_collection>;

// Parse inputs of known types. Instantiations are provided for geojson, geometry, feature, and
// feature_collection.
template <class T>
T parse(const std::string &);

// Parse any GeoJSON type.
geojson parse(const std::string &);

// Stringify inputs of known types. Instantiations are provided for geojson, geometry, feature, and
// feature_collection.
template <class T>
std::string stringify(const T &);

// Stringify any GeoJSON type.
std::string stringify(const geojson &);

} // namespace geojson
} // namespace maplibre
