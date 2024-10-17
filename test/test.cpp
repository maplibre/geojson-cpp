#include <maplibre/geojson.hpp>
#include <maplibre/geojson/rapidjson.hpp>
#include <maplibre/geometry.hpp>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace maplibre::geojson;

template <typename T = geojson>
geojson readGeoJSON(const std::string &path, bool use_convert) {
    std::ifstream t(path.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    if (use_convert) {
        rapidjson_document d;
        d.Parse<0>(buffer.str().c_str());
        return convert<T>(d);
    } else {
        return parse(buffer.str());
    }
}

template <class T>
std::string writeGeoJSON(const T &t, bool use_convert) {
    if (use_convert) {
        rapidjson_allocator allocator;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        convert(t, allocator).Accept(writer);
        return buffer.GetString();
    } else {
        return stringify(t);
    }
}

static void testEmpty() {
    const auto data = readGeoJSON<geometry>("test/fixtures/null.json", true);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<empty>(geom));

    assert(writeGeoJSON(geom, true) == "null");
}

static void testPoint(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/point.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<point>(geom));

    const auto &p = std::get<point>(geom);
    assert(p.x == 30.5);
    assert(p.y == 50.5);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testMultiPoint(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/multi-point.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<multi_point>(geom));

    const auto &points = std::get<multi_point>(geom);
    assert(points.size() == 2);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testLineString(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/line-string.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<line_string>(geom));

    const auto &points = std::get<line_string>(geom);
    assert(points.size() == 2);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testMultiLineString(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/multi-line-string.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<multi_line_string>(geom));

    const auto &lines = std::get<multi_line_string>(geom);
    assert(lines.size() == 1);
    assert(lines[0].size() == 2);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testPolygon(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/polygon.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<polygon>(geom));

    const auto &rings = std::get<polygon>(geom);
    assert(rings.size() == 1);
    assert(rings[0].size() == 5);
    assert(rings[0][0] == rings[0][4]);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testMultiPolygon(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/multi-polygon.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<multi_polygon>(geom));

    const auto &polygons = std::get<multi_polygon>(geom);
    assert(polygons.size() == 1);
    assert(polygons[0].size() == 1);
    assert(polygons[0][0].size() == 5);
    assert(polygons[0][0][0] == polygons[0][0][4]);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testGeometryCollection(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/geometry-collection.json", use_convert);
    assert(std::holds_alternative<geometry>(data));

    const auto &geom = std::get<geometry>(data);
    assert(std::holds_alternative<geometry_collection>(geom));

    const auto &collection = std::get<geometry_collection>(geom);
    assert(std::holds_alternative<point>(collection[0]));
    assert(std::holds_alternative<line_string>(collection[1]));

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testFeature(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/feature.json", use_convert);
    assert(std::holds_alternative<feature>(data));

    const auto &f = std::get<feature>(data);
    assert(std::holds_alternative<point>(f.geometry));

    assert(std::holds_alternative<bool>(f.properties.at("bool")));
    assert(f.properties.at("bool") == true);
    assert(std::holds_alternative<std::string>(f.properties.at("string")));
    assert(std::get<std::string>(f.properties.at("string")) == "foo");
    assert(f.properties.at("double") == 2.5);
    assert(std::holds_alternative<double>(f.properties.at("double")));
    assert(std::get<std::uint64_t>(f.properties.at("uint")) == 10);
    assert(std::holds_alternative<std::uint64_t>(f.properties.at("uint")));
    assert(std::get<std::int64_t>(f.properties.at("int")) == -10);
    assert(std::holds_alternative<std::int64_t>(f.properties.at("int")));
    assert(std::holds_alternative<maplibre::feature::null_value_t>(f.properties.at("null")));
    assert(f.properties.at("null") == maplibre::feature::null_value_t{});

    using prop_map = std::unordered_map<std::string, value>;

    const auto &nested = f.properties.at("nested");

    assert(std::holds_alternative<value::array_ptr_type>(nested));
    assert(std::holds_alternative<std::uint64_t>(std::get<value::array_ptr_type>(nested)->at(0)));
    assert(std::get<std::uint64_t>(std::get<value::array_ptr_type>(nested)->at(0)) == 5);
    assert(std::holds_alternative<value::object_ptr_type>(std::get<value::array_ptr_type>(nested)->at(1)));
    assert(std::holds_alternative<std::string>(
        std::get<value::object_ptr_type>(std::get<value::array_ptr_type>(nested)->at(1))->at("foo")));
    assert(std::get<std::string>(
               std::get<value::object_ptr_type>(std::get<value::array_ptr_type>(nested)->at(1))->at("foo")) == "bar");

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testFeatureNullProperties(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/feature-null-properties.json", use_convert);
    assert(std::holds_alternative<feature>(data));

    const auto &f = std::get<feature>(data);
    assert(std::holds_alternative<point>(f.geometry));
    assert(f.properties.size() == 0);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testFeatureNullGeometry(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/feature-null-geometry.json", use_convert);
    assert(std::holds_alternative<feature>(data));

    const auto &f = std::get<feature>(data);
    assert(std::holds_alternative<empty>(f.geometry));
    assert(f.properties.size() == 0);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testFeatureMissingProperties(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/feature-missing-properties.json", use_convert);
    assert(std::holds_alternative<feature>(data));

    const auto &f = std::get<feature>(data);
    assert(std::holds_alternative<point>(f.geometry));
    assert(f.properties.size() == 0);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testFeatureCollection(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/feature-collection.json", use_convert);
    assert(std::holds_alternative<feature_collection>(data));

    const auto &features = std::get<feature_collection>(data);
    assert(features.size() == 2);

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testFeatureID(bool use_convert) {
    const auto data = readGeoJSON("test/fixtures/feature-id.json", use_convert);
    assert(std::holds_alternative<feature_collection>(data));

    const auto &features = std::get<feature_collection>(data);

    assert(features.at(0).id == identifier{ uint64_t(1234) });
    assert(features.at(1).id == identifier{ "abcd" });

    assert(parse(writeGeoJSON(data, use_convert)) == data);
}

static void testParseErrorHandling() {
    try {
        readGeoJSON("test/fixtures/invalid.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("Invalid") != std::string::npos);
    }

    // test invalid polygon
    try {
        readGeoJSON("test/fixtures/invalid-polygon.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("described by 4") != std::string::npos);
    }

    try {
        readGeoJSON("test/fixtures/invalid-polygon-2.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("array") != std::string::npos);
    }

    try {
        readGeoJSON("test/fixtures/invalid-multi-polygon.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("described by 4") != std::string::npos);
    }

    try {
        readGeoJSON("test/fixtures/invalid-multi-polygon-2.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("2 numbers") != std::string::npos);
    }

    try {
        readGeoJSON("test/fixtures/invalid-line-string.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("two or more") != std::string::npos);
    }

    try {
        readGeoJSON("test/fixtures/invalid-multi-line-string.json", false);
        assert(false && "Should have thrown an error");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("two or more") != std::string::npos);
    }
}

void testAll(bool use_convert) {
    testPoint(use_convert);
    testMultiPoint(use_convert);
    testLineString(use_convert);
    testMultiLineString(use_convert);
    testPolygon(use_convert);
    testMultiPolygon(use_convert);
    testGeometryCollection(use_convert);
    testFeature(use_convert);
    testFeatureNullProperties(use_convert);
    testFeatureNullGeometry(use_convert);
    testFeatureMissingProperties(use_convert);
    testFeatureCollection(use_convert);
    testFeatureID(use_convert);
}

int main() {
    testParseErrorHandling();
    testEmpty();
    testAll(true);
    testAll(false);
    return 0;
}
