#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include "model.h"

class GLWidget;

class GLObject
{

    public:

        enum PaintAction
        {
            Initialize = 1 << 0,
            Draw       = 1 << 1,
            Finalize   = 1 << 2
        };
        typedef int PaintActionMask;

    protected:

        //! Pointer to parent model object.
        const Model *pParentModel;
        //! OpenGL widget.
        GLWidget *glWidget;
        //! Indicating whether environment settings should be applied.
        bool applyEnvSettings;
        //! Use OpenGL list.
        bool useGlList;
        //! Use GL_CULL_FACE.
        bool useGlCullFace;

    private:

        //! Internal initialization function.
        void _init ( const GLObject *pGlObject = nullptr );

    public:

        //! Constructor.
        explicit GLObject(GLWidget *glWidget);

        //! Copy constructor.
        GLObject(const GLObject &glObject);

        //! Destructor.
        ~GLObject();

        //! Assignment operator.
        GLObject & operator = (const GLObject &glObject);

        //! Return pointer to GLWidget.
        GLWidget * getGLWidget() const;

        //! Set pointer to parent model.
        void setParentModel(const Model *pParentModel);

        //! Return information on whether the environment settings should be applied.
        bool getApplyEnvironmentSettings() const;

        //! Set whether the environment settings should be applied.
        void setApplyEnvironmentSettings(bool applyEnvSettings);

        //! Return information on whether to use OpenGL list.
        bool getUseGlList() const;

        //! Set whether to use OpenGL list.
        void setUseGlList(bool useGlList);

        //! Return information on whether to use OpenGL cull face.
        bool getUseGlCullFace() const;

        //! Set whether to use OpenGL cull face.
        void setUseGlCullFace(bool useGlCullFace);

        //! Paint object.
        void paint(PaintActionMask paintActionMask = Initialize | Draw | Finalize);

        //! Set normal - wrapper arround (glNormal3d)
        static void glNormalVector(const RR3Vector &vector);

        //! Draw vertex - wrapper arround (glVertex3d)
        static void glVertexVector(const RR3Vector &vector);

        //! Draw node - wrapper arround (glVertex3d)
        static void glVertexNode(const RNode &node);

    protected:

        //! Initialize scene.
        virtual void initialize();

        //! Finalize scene.
        virtual void finalize();

        //! Draw scene.
        virtual void draw();

};

#endif /* GL_OBJECT_H */
