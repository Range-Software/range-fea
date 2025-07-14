#ifndef DRAW_ENGINE_H
#define DRAW_ENGINE_H

#include <QObject>
#include <QList>

#include "draw_engine_object.h"
#include "draw_engine_point.h"
#include "draw_engine_line.h"
#include "draw_engine_triangle.h"
#include "draw_engine_quadrilateral.h"
#include "draw_engine_circle.h"
#include "draw_engine_ellipse.h"
#include "draw_engine_cylinder.h"
#include "draw_engine_sphere.h"
#include "draw_engine_tetrahedron.h"
#include "draw_engine_hexahedron.h"
#include "draw_engine_raw.h"

class DrawEngine : public QObject
{

    Q_OBJECT

    protected:

        //! Engine is running.
        bool isRunning;
        //! Stack of drawing options.
        QList<DrawEngineObject*> objects;

    public:

        //! Constructor.
        explicit DrawEngine(QObject *parent = nullptr);

        //! Start engine.
        void start();

        //! Stop engine.
        void stop();

        //! Process all objects.
        void processObjects(const QList<uint> &modelIDs, bool mergeNearNodes, double tolerance = 0.0, bool findNearest = false);

        //! Add object.
        void addObject(DrawEngineObject *object);

        //! Remove object with given position.
        void removeObject(uint position);

        //! Return number of objects.
        uint getNObjects() const;

        //! Return const pointer to object at given position.
        const DrawEngineObject *getObject(uint position) const;

        //! Return pointer to object at given position.
        DrawEngineObject *getObject(uint position);

        //! Force to emit signal that object has changed.
        void setObjectChanged(uint position);

        //! Find size limits.
        bool findLimits(RLimitBox &limitBox) const;


    signals:

        //! Object was added signal.
        void objectAdded();

        //! Object at given position was removed signal.
        void objectRemoved(uint position);

        //! All object were removed signal.
        void objectsRemoved();

        //! Object at given position has changed.
        void objectChanged(uint position);

};

#endif // DRAW_ENGINE_H
