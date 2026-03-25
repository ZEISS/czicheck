// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <emscripten.h>
#include <emscripten/bind.h>

#include "blob_stream.h"
#include "czi_stream_adapter.h"
#include "wasm_runchecks.h"
#include "wasm_log.h"

#include "checkerfactory.h"
#include "checks.h"
#include "resultgathereroptions.h"
#include "resultgathererfactory.h"

#ifdef USE_WEB_PTHREADS
#include "proxied_blob_stream.h"
#endif

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <string>
#include <vector>
#include <memory>
#include <cstring>

// ---------------------------------------------------------------------------
// Helper: parse the config JSON from JS into a RunChecksConfig.
//
// Expected JSON shape:
// {
//   "checks": ["consistentcoordinates", "benabled", ...],    // optional, default = all non-opt-in
//   "laxParsing": false,                                     // optional
//   "ignoreSizeM": false,                                    // optional
//   "maxFindings": 3,                                        // optional, -1 = unlimited
//   "failFast": "none" | "checker" | "all"                   // optional
// }
// ---------------------------------------------------------------------------
static wasm::RunChecksConfig ParseConfig(const std::string& config_json,
                                         std::shared_ptr<ILog> log,
                                         std::uint64_t file_size)
{
    wasm::RunChecksConfig cfg;
    cfg.totalFileSize = file_size;
    cfg.gathererOptions.log = log;
    cfg.outputFormat = OutputEncodingFormat::JSON;

    if (config_json.empty())
        return cfg;

    rapidjson::Document doc;
    doc.Parse(config_json.c_str());
    if (doc.HasParseError() || !doc.IsObject())
        return cfg;

    if (doc.HasMember("checks") && doc["checks"].IsArray())
    {
        for (auto& v : doc["checks"].GetArray())
        {
            if (!v.IsString()) continue;
            CZIChecks check{};
            if (CCheckerFactory::TryParseShortName(v.GetString(), check))
            {
                cfg.checksToRun.push_back(check);
            }
        }
    }

    if (doc.HasMember("laxParsing") && doc["laxParsing"].IsBool())
        cfg.laxParsing = doc["laxParsing"].GetBool();

    if (doc.HasMember("ignoreSizeM") && doc["ignoreSizeM"].IsBool())
        cfg.ignoreSizeMForPyramidSubblocks = doc["ignoreSizeM"].GetBool();

    if (doc.HasMember("maxFindings") && doc["maxFindings"].IsInt())
        cfg.gathererOptions.maxNumberOfFindingsToPrint = doc["maxFindings"].GetInt();

    if (doc.HasMember("failFast") && doc["failFast"].IsString())
    {
        std::string ff = doc["failFast"].GetString();
        if (ff == "checker")
            cfg.gathererOptions.failFastMode = ResultGathererOptions::FailFastMode::FailFastForFatalErrorsPerChecker;
        else if (ff == "all")
            cfg.gathererOptions.failFastMode = ResultGathererOptions::FailFastMode::FailFastForFatalErrorsOverall;
    }

    return cfg;
}

// ---------------------------------------------------------------------------
// Exports
// ---------------------------------------------------------------------------

/// Returns a JSON array describing all available checkers.
/// Each element: { "shortName": "...", "displayName": "...", "isOptIn": bool }
static std::string GetCheckersJson()
{
    rapidjson::Document doc;
    doc.SetArray();
    auto& alloc = doc.GetAllocator();

    CCheckerFactory::EnumerateCheckers(
        [&](const CCheckerFactory::CheckersInfo& info) -> bool
        {
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("shortName",
                          rapidjson::Value().SetString(info.shortName.c_str(), alloc), alloc);
            obj.AddMember("displayName",
                          rapidjson::Value().SetString(info.displayName.c_str(), alloc), alloc);
            obj.AddMember("isOptIn", info.isOptIn, alloc);
            doc.PushBack(obj, alloc);
            return true;
        });

    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    doc.Accept(writer);
    return std::string(buf.GetString(), buf.GetSize());
}

/// Run CZI checks on a JS File/Blob.
///
/// \param js_handle   Index into Module._fileHandles[] on the JS side.
/// \param file_size   Size of the file in bytes.
/// \param config_json JSON string with run configuration (see ParseConfig).
///
/// \returns JSON string with the checker results.
static std::string RunChecksOnBlob(int js_handle, double file_size, const std::string& config_json)
{
    const auto size = static_cast<std::uint64_t>(file_size);

#ifdef USE_WEB_PTHREADS
    auto wasm_stream = wasm::make_proxied_blob_stream(js_handle, size);
#else
    auto wasm_stream = wasm::make_blob_stream(js_handle, size);
#endif

    auto adapter = std::make_shared<wasm::CziStreamAdapter>(*wasm_stream);

    auto log = std::make_shared<wasm::StringLog>();
    auto cfg = ParseConfig(config_json, log, size);

    IResultGatherer::AggregatedResult aggregated{};
    bool ok = wasm::RunChecks(adapter, cfg, aggregated);

    if (!ok)
    {
        std::string err = log->GetStdErr();
        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();
        doc.AddMember("error", true, alloc);
        doc.AddMember("message",
                      rapidjson::Value().SetString(err.c_str(), alloc), alloc);
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        doc.Accept(writer);
        return std::string(buf.GetString(), buf.GetSize());
    }

    return log->GetStdOut();
}

// ---------------------------------------------------------------------------
// JS Embinds
// ---------------------------------------------------------------------------
EMSCRIPTEN_BINDINGS(czicheck)
{
    emscripten::function("getCheckers", &GetCheckersJson);
    emscripten::function("runChecks", &RunChecksOnBlob);
}

// We still need this iirc
int main() { return 0; }
