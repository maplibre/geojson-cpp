#pragma once

#include <maplibre/geojson.hpp>
#include <maplibre/geojson/value.hpp>

#include <cassert>
#include <variant>

namespace maplibre {
namespace geojson {

using error = std::runtime_error;

namespace {

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

double getDouble(const value &numVal) {
    return std::visit(
        overloaded{ [](double n) -> double { return n; }, [](uint64_t n) -> double { return n; },
                    [](int64_t n) -> double { return n; },
                    [](const auto &) -> double { throw error("coordinate's value must be of a Number type"); } },
        numVal);
}

} // namespace

template <typename T>
T convert(const value &);

template <>
point convert<point>(const value &val) {
    assert(std::holds_alternative<std::shared_ptr<std::vector<value>>>(val));
    if (!std::holds_alternative<std::shared_ptr<std::vector<value>>>(val)) {
        throw error("coordinates must be of an Array type");
    }

    const auto &valuePoint = *val.getArray();
    if (valuePoint.size() < 2) {
        throw error("coordinates array must have at least 2 numbers");
    }

    return point{ getDouble(valuePoint[0]), getDouble(valuePoint[1]) };
}

template <typename Container>
Container convert(const value &val) {
    assert(std::holds_alternative<std::shared_ptr<std::vector<value>>>(val));
    if (!std::holds_alternative<std::shared_ptr<std::vector<value>>>(val)) {
        throw error("coordinates must be of an Array type");
    }

    const auto &pointArray = *val.getArray();
    Container points;
    points.reserve(pointArray.size());
    for (const auto &p : pointArray) {
        points.push_back(convert<typename Container::value_type>(p));
    }
    return points;
}

template <>
geometry convert<geometry>(const value &val) {
    auto valueObject = val.getObject();
    if (!valueObject) {
        throw error("GeoJSON must be an object");
    }

    auto typeIt = valueObject->find("type");
    if (typeIt == valueObject->end()) {
        throw error("Geometry must have a type property");
    }

    const auto &typeValue = typeIt->second;
    if (!std::holds_alternative<std::string>(typeValue)) {
        throw error("Geometry 'type' property must be of a String type");
    }

    const auto &typeString = *typeValue.getString();

    if (typeString == "GeometryCollection") {
        auto geometriesIt = valueObject->find("geometries");
        if (geometriesIt == valueObject->end()) {
            throw error("GeometryCollection must have a geometries property");
        }

        const auto geometryArray = geometriesIt->second.getArray();
        if (!geometryArray) {
            throw error("GeometryCollection geometries property must be an array");
        }

        return geometry{ convert<geometry_collection>(*geometryArray) };
    }

    auto coordinatesIt = valueObject->find("coordinates");
    if (coordinatesIt == valueObject->end()) {
        throw error(typeString + " geometry must have a coordinates property");
    }

    const auto coordinateArray = coordinatesIt->second.getArray();
    if (!coordinateArray) {
        throw error("coordinates property must be an array");
    }

    if (typeString == "Point")
        return geometry{ convert<point>(*coordinateArray) };
    if (typeString == "MultiPoint")
        return geometry{ convert<multi_point>(*coordinateArray) };
    if (typeString == "LineString")
        return geometry{ convert<line_string>(*coordinateArray) };
    if (typeString == "MultiLineString")
        return geometry{ convert<multi_line_string>(*coordinateArray) };
    if (typeString == "Polygon")
        return geometry{ convert<polygon>(*coordinateArray) };
    if (typeString == "MultiPolygon")
        return geometry{ convert<multi_polygon>(*coordinateArray) };

    throw error(typeString + " not yet implemented");
}

template <>
feature convert<feature>(const value &val) {
    auto valueObject = val.getObject();
    if (!valueObject) {
        throw error("GeoJSON must be an object");
    }

    auto typeIt = valueObject->find("type");
    if (typeIt == valueObject->end()) {
        throw error("Feature must have a type property");
    }

    const auto &typeValue = typeIt->second;
    if (!std::holds_alternative<std::string>(typeValue)) {
        throw error("Feature 'type' property must be of a String type");
    }

    if (*typeValue.getString() != "Feature") {
        throw error("Feature type must be Feature");
    }

    auto geometryIt = valueObject->find("geometry");
    if (geometryIt == valueObject->end()) {
        throw error("Feature must have a geometry property");
    }

    feature result{ convert<geometry>(geometryIt->second) };
    auto idIt = valueObject->find("id");
    if (idIt != valueObject->end()) {
        result.id = std::visit(
            overloaded{ [](const std::string &string) -> identifier { return { string }; },
                        [](int64_t number) -> identifier { return { number }; },
                        [](uint64_t number) -> identifier { return { number }; },
                        [](double number) -> identifier { return { number }; },
                        [](const auto &) -> identifier { throw error("Feature id must be a string or number"); } },
            idIt->second);
    }

    auto propertiesIt = valueObject->find("properties");
    if (propertiesIt != valueObject->end() &&
        !std::holds_alternative<maplibre::geojson::null_value_t>(propertiesIt->second)) {
        if (!std::holds_alternative<value::object_ptr_type>(propertiesIt->second)) {
            throw error("properties must be an object");
        }
        result.properties = *propertiesIt->second.getObject();
    }

    return result;
}

template <>
geojson convert<geojson>(const value &val) {
    auto valueObject = val.getObject();
    if (!valueObject) {
        throw error("GeoJSON must be an object");
    }

    auto typeIt = valueObject->find("type");
    if (typeIt == valueObject->end()) {
        throw error("GeoJSON must have a type property");
    }

    const auto &typeValue = typeIt->second;
    if (!std::holds_alternative<std::string>(typeValue)) {
        throw error("GeoJSON 'type' property must be of a String type");
    }

    const auto &typeString = *typeValue.getString();
    if (typeString == "FeatureCollection") {
        auto featuresIt = valueObject->find("features");
        if (featuresIt == valueObject->end()) {
            throw error("FeatureCollection must have features property");
        }

        const auto featureArray = featuresIt->second.getArray();
        if (!featureArray) {
            throw error("FeatureCollection features property must be an array");
        }

        feature_collection collection;
        collection.reserve(featureArray->size());
        for (const auto &featureValue : *featureArray) {
            collection.push_back(convert<feature>(featureValue));
        }
        return geojson{ collection };
    }

    if (typeString == "Feature") {
        return geojson{ convert<feature>(val) };
    }

    return geojson{ convert<geometry>(val) };
}

template feature_collection convert<feature_collection>(const value &);

geojson convert(const value &val) {
    return std::visit(
        overloaded{ [](const null_value_t &) -> geojson { return geometry{}; },
                    [](const std::string &jsonString) { return jsonString == "null" ? geometry{} : parse(jsonString); },
                    [](const value::object_type &jsonObject) {
                        return convert<geojson>(static_cast<const maplibre::geojson::value &>(jsonObject));
                    },
                    [](const value::object_ptr_type obj) { return convert<geojson>(*obj); },
                    [](const auto &) -> geojson { throw error("Invalid GeoJSON value was provided."); } },
        val);
}

value convert(const point &p) {
    return value::array_type{ p.x, p.y };
}

template <typename Cont>
value convert(const Cont &points) {
    value::array_type result;
    result.reserve(points.size());
    for (const auto &p : points) {
        result.emplace_back(convert(p));
    }
    return result;
}

template <>
value convert(const geometry &geom) {
    return std::visit(
        overloaded{ [](const empty &) { return value{}; },
                    [](const point &p) -> value {
                        return value::object_type{ { "type", "Point" }, { "coordinates", convert(p) } };
                    },
                    [](const multi_point &mp) -> value {
                        return value::object_type{ { "type", "MultiPoint" }, { "coordinates", convert(mp) } };
                    },
                    [](const line_string &ls) -> value {
                        return value::object_type{ { "type", "LineString" }, { "coordinates", convert(ls) } };
                    },
                    [](const multi_line_string &mls) -> value {
                        return value::object_type{ { "type", "MultiLineString" }, { "coordinates", convert(mls) } };
                    },
                    [](const polygon &pol) -> value {
                        return value::object_type{ { "type", "Polygon" }, { "coordinates", convert(pol) } };
                    },
                    [](const multi_polygon &mpol) -> value {
                        return value::object_type{ { "type", "MultiPolygon" }, { "coordinates", convert(mpol) } };
                    },
                    [](const geometry_collection &gc) -> value {
                        value::array_type geometries;
                        geometries.reserve(gc.size());
                        for (const auto &gcGeom : gc) {
                            geometries.push_back(convert(gcGeom));
                        }
                        return value::object_type{ { "type", "GeometryCollection" },
                                                   { "geometries", std::move(geometries) } };
                    } },
        geom);
}

template <>
value convert(const feature &f) {
    value::object_type result{ { "type", "Feature" },
                               { "geometry", convert(f.geometry) },
                               { "properties", f.properties } };

    if (!std::holds_alternative<maplibre::geojson::null_value_t>(f.id)) {
        value id =
            std::visit(overloaded{ [](uint64_t n) -> value { return n; }, [](int64_t n) -> value { return n; },
                                   [](double n) -> value { return n; }, [](std::string s) -> value { return s; },
                                   [](const auto &) -> value { throw error("Unknown type for a Feature 'id'"); } },
                       f.id);
        result.emplace(std::make_pair("id", std::move(id)));
    }

    return result;
}

template <>
value convert(const feature_collection &collection) {
    value::object_type result{ { "type", "FeatureCollection" } };
    value::array_type features;
    features.reserve(collection.size());
    for (const auto &feat : collection) {
        features.emplace_back(convert(feat));
    }
    result.emplace(std::make_pair("features", std::move(features)));
    return result;
}

template <>
value convert(const geojson &json) {
    return convert(json);
}

value convert(const geojson &json) {
    return std::visit(overloaded{ [](const geometry &g) -> value { return convert(g); },
                                  [](const feature &f) -> value { return convert(f); },
                                  [](const feature_collection &c) -> value { return convert(c); } },
                      json);
}

} // namespace geojson
} // namespace maplibre
