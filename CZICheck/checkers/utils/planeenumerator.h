// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <utility>
#include <stdexcept>

#include <libCZI.h>

/// This will enumerate all the planes in a source-document. 
/// * The enumeration is done "per scene"
/// * The enumeration gives a plane-coordinate for each plane.
/// * If the dim-bounds is empty (no valid dimensions), a single element with an empty coordinate is yielded.
/// * The order in which the plane coordinates are incremented is: S, C, Z, T, R, I, H, V, B.
class PlaneEnumerator
{
private:
    libCZI::SubBlockStatistics sub_block_statistics_;
public:
    /// This is the element we enumerate.
    struct PlaneRegion
    {
        /// The plane coordinate. Note that this may or may not contain the S-index (depending on the source document).
        libCZI::CDimCoordinate plane_coordinate;

        /// Query if the coordinate (in the field 'plane_coordinate') contains a value for the S-index (or scene-index).
        ///
        /// \returns    True if 'plane_coordinate' contains an S-index; false if not.
        bool ContainsSceneIndex() const
        {
            return this->plane_coordinate.TryGetPosition(libCZI::DimensionIndex::S, nullptr);
        }
    };
private:
    /// This defines the order of dimensions to iterate over.
    static constexpr std::array<libCZI::DimensionIndex, 9> kOrderOfDimensionsToIterate =
    {
        libCZI::DimensionIndex::S, libCZI::DimensionIndex::C, libCZI::DimensionIndex::Z, libCZI::DimensionIndex::T,
        libCZI::DimensionIndex::R, libCZI::DimensionIndex::I, libCZI::DimensionIndex::H, libCZI::DimensionIndex::V,
        libCZI::DimensionIndex::B
    };

    static_assert(
        kOrderOfDimensionsToIterate.size() ==
            (static_cast<std::size_t>(libCZI::DimensionIndex::MaxDim) - static_cast<std::size_t>(libCZI::DimensionIndex::MinDim) + 1U),
            "kOrderOfDimensionsToIterate must contain exactly one entry per valid DimensionIndex.");

    const libCZI::CDimBounds& GetDimBounds() const
    {
        return this->sub_block_statistics_.dimBounds;
    }
public:
    PlaneEnumerator() = delete;

    explicit PlaneEnumerator(const libCZI::SubBlockStatistics& sub_block_statistics) : sub_block_statistics_(sub_block_statistics)
    {
    }

    /// Forward declaration — see Sentinel below.
    struct Sentinel;

    /// Nested class for the iterator.
    class Iterator final
    {
    private:
        const PlaneEnumerator& plane_enumerator_;
        libCZI::CDimCoordinate current_state_;
        bool is_end_ = false;
    protected:
        /// We restrict the constructor to the PlaneEnumerator-class only (by making it protected).
        ///
        /// \param  plane_enumerator The plane enumerator object (and - we assume its lifetime to be greater than ours).
        /// \param  start_coordinate The start coordinate.
        explicit Iterator(
            const PlaneEnumerator& plane_enumerator,
            libCZI::CDimCoordinate start_coordinate) :
            plane_enumerator_(plane_enumerator),
            current_state_(std::move(start_coordinate))
        {
        }

        friend class PlaneEnumerator;
    public:
        Iterator() = delete;

        /// Prefix increment operator.
        ///
        /// \returns The result of the operation.
        Iterator& operator++()
        {
            if (this->is_end_)
            {
                return *this;
            }

            for (auto dimension : PlaneEnumerator::kOrderOfDimensionsToIterate)
            {
                int coordinate;
                if (this->current_state_.TryGetPosition(dimension, &coordinate))
                {
                    int start, size;
                    this->plane_enumerator_.GetDimBounds().TryGetInterval(dimension, &start, &size);
                    if (coordinate < start + size - 1)
                    {
                        this->current_state_.Set(dimension, coordinate + 1);
                        return *this;
                    }
                    else
                    {
                        this->current_state_.Set(dimension, start);
                    }
                }
            }

            // If we get here, we have overflowed (or the dim-bounds was empty to begin with).
            this->is_end_ = true;

            return *this;
        }

        // Dereference operator
        PlaneRegion operator*() const
        {
            PlaneRegion result;
            result.plane_coordinate = this->current_state_;
            return result;
        }

        /// Comparison against the sentinel (end marker).
        ///
        /// \param  The sentinel value returned by PlaneEnumerator::end().
        ///
        /// \returns True if this iterator has not yet reached the end.
        bool operator!=(const Sentinel&) const
        {
            return !this->is_end_;
        }
    };

    /// Sentinel type returned by end(). Using a distinct type avoids ambiguous
    /// iterator-vs-iterator comparisons and makes range-for the only supported
    /// iteration pattern.
    struct Sentinel {};

    /// Begin iterator.
    ///
    /// \returns An Iterator.
    Iterator begin() const
    {
        libCZI::CDimCoordinate start_coordinate;
        const libCZI::CDimBounds& dimension_bounds = this->GetDimBounds();
        dimension_bounds.EnumValidDimensions(
            [&](libCZI::DimensionIndex dim, int start, int size)->bool
            {
                start_coordinate.Set(dim, start);
                return true;
            });

        return Iterator(*this, start_coordinate);
    }

    /// End sentinel.
    ///
    /// \returns A Sentinel value that compares not-equal to any non-exhausted Iterator.
    Sentinel end() const
    {
        return Sentinel{};
    }
private:
};
