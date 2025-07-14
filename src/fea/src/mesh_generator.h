#ifndef MESH_GENERATOR_H
#define MESH_GENERATOR_H


#include <QObject>

#include <rbl_job.h>

class MeshGenerator : public RJob
{
    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;

    public:

        //! Constructor.
        explicit MeshGenerator(uint modelID);

    protected:

        //! Overloaded run method.
        virtual int perform() override final;

};

#endif // MESH_GENERATOR_H
