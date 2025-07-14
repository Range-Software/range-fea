#ifndef GL_NODE_H
#define GL_NODE_H

#include "gl_object.h"

class GLNode : public GLObject, public RNode
{

    protected:

        // Environment settings
        GLboolean lightingEnabled;
        GLfloat pointSize;

    private:

        //! Internal initialization function.
        void _init(const GLNode *pGlNode = nullptr);

    public:

        //! Constructor.
        GLNode(GLWidget *glWidget);

        //! Copy constructor.
        GLNode(const GLNode &glNode);

        //! Destructor.
        ~GLNode();

        //! Assignment operator.
        GLNode & operator = (const GLNode &glNode);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif /* GL_NODE_H */
