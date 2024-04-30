// SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <memory>
#include <string>
#include "IChecker.h"
#include "utils.h"
#include "checkerfactory.h"
#include "checkers/checkerSubBlkDirPositions.h"
#include "checkers/checkerSubBlkSegmentsValid.h"
#include "checkers/checkerConsistentCoordinates.h"
#include "checkers/checkerDuplicateCoordinates.h"
#include "checkers/checkerBenabled.h"
#include "checkers/checkerSamePixeltypePerChannel.h"
#include "checkers/checkerXmlMetadataXsdValidation.h"
#include "checkers/checkerPlanesStartIndices.h"
#include "checkers/checkerConsecutivePlaneIndices.h"
#include "checkers/checkerMissingMindex.h"
#include "checkers/checkerXmlBasicMetadataValidation.h"
#include "checkers/checkerOverlappingScenes.h"
#include "checkers/checkerSubBlkBitmapValid.h"
#include "checkers/checkerTopographyApplianceValidation.h"

using namespace std;

/// The information we store about a checker class, for implementing the
/// class factory functionality.
struct classEntry
{
    /// The enum identifying a checker class.
    CZIChecks check;

    /// A human readable display name identifying and describing the checker class.
    const string displayname;

    /// A short name identifying the checker class. This string has to be unique.
    const string shortname;

    /// A flag to indicate that the checker won't be executed by default, but it has to be explicitly opted in for.
    bool isOptIn;

    /// A function pointer which creates a new instance of the respective checker class.
    std::unique_ptr<IChecker>(*factory)(
        const std::shared_ptr<libCZI::ICZIReader>&,
        CResultGatherer&,
        const CheckerCreateInfo&);
};

template <typename T>
static std::unique_ptr<IChecker> createCheckerInstance(
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& resultGatherer,
    const CheckerCreateInfo& additionalInfo)
{
    return unique_ptr<T>{new T(reader, resultGatherer, additionalInfo)};
}

template <typename T>
constexpr classEntry MakeEntry(bool isOptIn)
{
    return classEntry
    {
        T::kCheckType,
        T::kDisplayName,
        T::kShortName,
        isOptIn,
        &createCheckerInstance<T>
    };
}

template <typename T>
constexpr classEntry MakeEntry()
{
    return classEntry
    {
        T::kCheckType,
        T::kDisplayName,
        T::kShortName,
        false,
        &createCheckerInstance<T>
    };
}

/// The repository where we list all available checkers.
static const classEntry classesList[] =
{
    MakeEntry<CCheckConsistentCoordinates>(),
    MakeEntry<CCheckSubBlkDirPositions>(),
    MakeEntry<CCheckSubBlkSegmentsValid>(true), // we make this "opt-in" because "CCheckSubBlkBitmapValid" includes the same check (and is more extensive)
    MakeEntry<CCheckDuplicateCoordinates>(),
    MakeEntry<CCheckBenabled>(),
    MakeEntry<CCheckSamePixeltypePerChannel>(),
    MakeEntry<CCheckPlanesStartIndices>(),
    MakeEntry<CCheckConsecutivePlaneIndices>(),
    MakeEntry<CCheckMissingMindex>(),
    MakeEntry<CCheckBasicMetadataValidation>(),
    MakeEntry<CCheckTopgraphyApplianceMetadata>(),
#if CZICHECK_XERCESC_AVAILABLE
    MakeEntry<CCheckXmlMetadataXsdValidation>(true),
#endif
    MakeEntry<CCheckOverlappingScenesOnLayer0>(),
    MakeEntry<CCheckSubBlkBitmapValid>(),
};

/*static*/std::unique_ptr<IChecker> CCheckerFactory::CreateChecker(
    CZIChecks check,
    const std::shared_ptr<libCZI::ICZIReader>& reader,
    CResultGatherer& result_gatherer,
    const CheckerCreateInfo& additional_info)
{
    for (const auto& c : classesList)
    {
        if (c.check == check)
        {
            return c.factory(reader, result_gatherer, additional_info);
        }
    }

    return {};
}

/*static*/const std::string& CCheckerFactory::GetCheckerDisplayName(CZIChecks check_type)
{
    static const string kUnknown{ ">unknown<" };

    for (const auto& c : classesList)
    {
        if (c.check == check_type)
        {
            return c.displayname;
        }
    }

    return kUnknown;
}

/*static*/bool CCheckerFactory::TryParseShortName(const string& short_name, CZIChecks& check_type)
{
    for (const auto& c : classesList)
    {
        if (icasecmp(c.shortname, short_name))
        {
            check_type = c.check;
            return true;
        }
    }

    return false;
}

/*static*/void CCheckerFactory::EnumerateCheckers(const std::function<bool(const CCheckerFactory::CheckersInfo&)>& enum_func)
{
    for (const auto& c : classesList)
    {
        CheckersInfo info;
        info.checkerType = c.check;
        info.shortName = c.shortname;
        info.displayName = c.displayname;
        info.isOptIn = c.isOptIn;
        if (!enum_func(info))
        {
            break;
        }
    }
}
