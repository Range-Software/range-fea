#ifndef GL_SCALAR_FIELD_H
#define GL_SCALAR_FIELD_H

#include <rml_scalar_field.h>

#include "gl_object.h"
#include "gl_texture.h"
#include "model.h"


class ScalarFieldItem;

class GLScalarField : public GLObject, public RScalarField
{

    protected:

        //! entity ID.
        SessionEntityID entityID;
        // Environment settings
        GLboolean lightingEnabled;
        GLboolean normalize;

    private:

        //! Internal initialization function.
        void _init(const GLScalarField *pGlScalarField = nullptr);

    public:

        //! Constructor.
        GLScalarField(GLWidget *glWidget, const RScalarField &scalarField, const SessionEntityID &entityID);

        //! Copy constructor.
        GLScalarField(const GLScalarField &glScalarField);

        //! Destructor.
        ~GLScalarField();

        //! Assignment operator.
        GLScalarField & operator = (const GLScalarField &glScalarField);

        //! Returtn entity ID.
        const SessionEntityID & getEntityID() const;

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

        //! Calculate the vector field.
        std::vector<ScalarFieldItem> calculateField(const RVariable *pScalarVariable, const RVariable *pDisplacementVariable) const;

        //! Return vector of scalar variable values.
        std::vector<double> getScalarValues(uint elementID, RVariableApplyType variableApplyType, const RVariable *pVariable) const;

        //! Return vector of displacement variable values.
        std::vector<RR3Vector> getDisplacementValues(uint elementID, RVariableApplyType variableApplyType, const RVariable *pVariable) const;

};

#endif // GL_SCALAR_FIELD_H
