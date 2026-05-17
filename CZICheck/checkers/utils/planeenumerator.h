// SPDX-FileCopyrightText: 2026 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <libCZI.h>

    /// This will enumerate all the planes in a source-document. 
    /// * The enumeration is done "per scene" (as the pyramids are constructed per scene).  
    /// * The enumeration gives a plane-coordinate and a bounding box. In case the source has a S-index,  
    ///    this bounding box is the bounding box of the scene. Otherwise, it is the bounding box of the document.
    /// * The order is (in which the plane coordinates are incremented) is: S, C, Z, T, R, I, H, V.
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

        /// The ROI (either for the scene, if an S-index is present), or for the whole plane.
        libCZI::IntRect rect;

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
    static constexpr std::array<libCZI::DimensionIndex, 8> kOrderOfDimensionsToIterate =
    {
        libCZI::DimensionIndex::S, libCZI::DimensionIndex::C, libCZI::DimensionIndex::Z, libCZI::DimensionIndex::T,
        libCZI::DimensionIndex::R, libCZI::DimensionIndex::I, libCZI::DimensionIndex::H, libCZI::DimensionIndex::V,
    };

    libCZI::IntRect GetRoiForDocument() const
    {
        return this->sub_block_statistics_.boundingBoxLayer0Only;
    }

    libCZI::IntRect GetRoiForScene(int s_index) const
    {
        return this->sub_block_statistics_.sceneBoundingBoxes.at(s_index).boundingBoxLayer0;
    }

    const libCZI::CDimBounds& GetDimBounds() const
    {
        return this->sub_block_statistics_.dimBounds;
    }
public:
    PlaneEnumerator() = delete;

    PlaneEnumerator(const libCZI::SubBlockStatistics& sub_block_statistics) : sub_block_statistics_(sub_block_statistics)
    {
        // requirements:
        // - dimBounds must contain at least one dimension (other than the scene dimension)
        // - if there is a scene dimension, the sceneBoundingBoxes must contain bounding boxes for all scenes
        bool other_dimension_than_scene_found = false;
        for (const auto dimension : PlaneEnumerator::kOrderOfDimensionsToIterate)
        {
            if (dimension != libCZI::DimensionIndex::S)
            {
                if (this->sub_block_statistics_.dimBounds.TryGetInterval(dimension, nullptr, nullptr))
                {
                    other_dimension_than_scene_found = true;
                    break;
                }
            }
        }

        if (!other_dimension_than_scene_found)
        {
            throw std::invalid_argument("The dimension bounds must contain at least one dimension (other than the scene dimension).");
        }

        int start_s, size_s;
        if (this->sub_block_statistics_.dimBounds.TryGetInterval(libCZI::DimensionIndex::S, &start_s, &size_s))
        {
            for (int i = start_s; i < start_s + size_s; ++i)
            {
                if (this->sub_block_statistics_.sceneBoundingBoxes.find(i) == this->sub_block_statistics_.sceneBoundingBoxes.end())
                {
                    throw std::invalid_argument("The scene bounding boxes must contain bounding boxes for all scenes.");
                }
            }
        }
    }

    /// Nested class for the iterator.
    class Iterator final
    {
    private:
        const PlaneEnumerator& plane_enumerator_;
        libCZI::CDimCoordinate current_state_;
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

            // If we get here, we have overflowed with the last coordinate (i.e. we have reached the end).
            // In this case we set the current state to the coordinate one after the end.
            this->current_state_ = this->plane_enumerator_.GetCoordinateOneAfterEnd();

            return *this;
        }

        // Dereference operator
        PlaneRegion operator*() const
        {
            PlaneRegion result;
            result.plane_coordinate = this->current_state_;

            int s_index;
            if (this->current_state_.TryGetPosition(libCZI::DimensionIndex::S, &s_index))
            {
                result.rect = this->plane_enumerator_.GetRoiForScene(s_index);
            }
            else
            {
                result.rect = this->plane_enumerator_.GetRoiForDocument();
            }

            return result;
        }

        /// Comparison operator.
        ///
        /// \param  other The other object to compare to.
        ///
        /// \returns True if the parameters are not considered equivalent.
        bool operator!=(const Iterator& other) const
        {
            int comparison_result = libCZI::Utils::Compare(&this->current_state_, &other.current_state_);
            return comparison_result != 0;
        }
    };

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

    /// End iterator (one past the last).
    ///
    /// \returns An Iterator pointing to "one after the last element".
    Iterator end() const
    {
        return Iterator(*this, this->GetCoordinateOneAfterEnd());
    }
private:

    /// Gets a coordinate "one after the end"
    ///
    /// \returns    The coordinate "one after the end".
    libCZI::CDimCoordinate GetCoordinateOneAfterEnd() const
    {
        libCZI::CDimCoordinate end_coordinate;
        const libCZI::CDimBounds& dimension_bounds = this->GetDimBounds();
        bool is_first_coordinate = true;
        for (auto dimension : PlaneEnumerator::kOrderOfDimensionsToIterate)
        {
            int start, size;
            if (dimension_bounds.TryGetInterval(dimension, &start, &size))
            {
                // set the first index to "one after the end", the rest to the max value
                if (is_first_coordinate)
                {
                    end_coordinate.Set(dimension, start + size);
                    is_first_coordinate = false;
                }
                else
                {
                    end_coordinate.Set(dimension, start + size - 1);
                }
            }
        }

        return end_coordinate;
    }
};

