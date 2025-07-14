#ifndef GEOMETRY_TRANSFORM_INPUT_H
#define GEOMETRY_TRANSFORM_INPUT_H

#include <rbl_r3vector.h>

class GeometryTransformInput
{

    public:

        enum ApplyTo
        {
            ApplyToAll = 0,
            ApplyToSelected,
            ApplyToPicked,
            ApplyToVisible
        };

    protected:

        //! Translation vector.
        RR3Vector translation;
        //! Rotation vector (angles in degrees).
        RR3Vector rotation;
        //! Rotation center.
        RR3Vector rotationCenter;
        //! Scale vector.
        RR3Vector scale;
        //! Scale center.
        RR3Vector scaleCenter;
        //! Apply to.
        ApplyTo applyTo;
        //! Include shared nodes.
        bool includeSharedNodes;
        //! Split shared nodes.
        bool splitSharedNodes;
        //! Sweep shared nodes.
        bool sweepSharedNodes;
        //! Number of sweep steps.
        uint nSweepSteps;

    private:

        //! Internal initialization function.
        void _init(const GeometryTransformInput *pGeometryTransformInput = nullptr);

    public:

        //! Constructor.
        GeometryTransformInput();

        //! Copy constructor.
        GeometryTransformInput(const GeometryTransformInput &geometryTransformInput);

        //! Destructor.
        ~GeometryTransformInput();

        //! Assignment operator.
        GeometryTransformInput & operator =(const GeometryTransformInput &geometryTransformInput);

        //! Return translation vector.
        const RR3Vector &getTranslation() const;

        //! Return translation vector.
        RR3Vector &getTranslation();

        //! Set translation vector.
        void setTranslation(const RR3Vector &translation);

        //! Return rotation vector.
        const RR3Vector &getRotation() const;

        //! Return rotation vector.
        RR3Vector &getRotation();

        //! Set rotation vector.
        void setRotation(const RR3Vector &rotation);

        //! Return rotation center.
        const RR3Vector &getRotationCenter() const;

        //! Return rotation center.
        RR3Vector &getRotationCenter();

        //! Set rotation center.
        void setRotationCenter(const RR3Vector &rotationCenter);

        //! Return scale vector.
        const RR3Vector &getScale() const;

        //! Return scale vector.
        RR3Vector &getScale();

        //! Set scale vector.
        void setScale(const RR3Vector &scale);

        //! Return scale center.
        const RR3Vector &getScaleCenter() const;

        //! Return scale center.
        RR3Vector &getScaleCenter();

        //! Set scale center.
        void setScaleCenter(const RR3Vector &scaleCenter);

        //! Return apply to.
        GeometryTransformInput::ApplyTo getApplyTo() const;

        //! Set apply to.
        void setApplyTo(const GeometryTransformInput::ApplyTo &applyTo);

        //! Return whether to include shared nodes.
        bool getIncludeSharedNodes() const;

        //! Set whether to include shared nodes.
        void setIncludeSharedNodes(bool includeSharedNodes);

        //! Return whether to split shared nodes.
        bool getSplitSharedNodes() const;

        //! Set whether to split shared nodes.
        void setSplitSharedNodes(bool splitSharedNodes);

        //! Return whether to perform sweep on split nodes.
        bool getSweepSharedNodes() const;

        //! Set whether to perform sweep on split nodes.
        void setSweepSharedNodes(bool sweepSharedNodes);

        //! Return number of sweep steps.
        uint getNSweepSteps() const;

        //! Set number of sweep steps.
        void setNSweepSteps(uint nSweepSteps);

        //! Return true if translation is active.
        bool isTranslateActive() const;

        //! Return true if rotation is active.
        bool isRotateActive() const;

        //! Return true if scaling is active.
        bool isScaleActive() const;

};

#endif // GEOMETRY_TRANSFORM_INPUT_H
