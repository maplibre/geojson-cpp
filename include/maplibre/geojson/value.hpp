#pragma once

#include <maplibre/geojson.hpp>

namespace maplibre {
namespace geojson {

// Convert Value to known types. Instantiations are provided for geojson, geometry, feature, and
// feature_collection.
template <class T>
T convert(const maplibre::geojson::value &);

// Converts Value to GeoJSON type.
geojson convert(const maplibre::geojson::value &);

// Convert inputs of known types to Value. Instantiations are provided for geojson, geometry, feature, and
// feature_collection.
template <class T>
maplibre::geojson::value convert(const T &);

// Converts GeoJSON type to Value.
maplibre::geojson::value convert(const geojson &);

} // namespace geojson
} // namespace maplibre
