#include "gl_functions.h"
#include "gl_cut_plane.h"
#include "gl_arrow.h"
#include "gl_widget.h"

void GLCutPlane::_init(const GLCutPlane *pGlCutPlane)
{
    if (pGlCutPlane)
    {
        this->size = pGlCutPlane->size;
    }
}

GLCutPlane::GLCutPlane(GLWidget *glWidget, const RPlane &plane)
    : GLObject(glWidget)
    , RPlane(plane)
    , size(1.0)
{
    this->_init();
}

GLCutPlane::GLCutPlane(const GLCutPlane &glCutPlane)
    : GLObject(glCutPlane)
    , RPlane(glCutPlane)
{
    this->_init(&glCutPlane);
}

GLCutPlane::~GLCutPlane()
{
}

GLCutPlane &GLCutPlane::operator =(const GLCutPlane &glCutPlane)
{
    this->GLObject::operator =(glCutPlane);
    this->RPlane::operator =(glCutPlane);
    this->_init(&glCutPlane);
    return (*this);
}

float GLCutPlane::getSize() const
{
    return this->size;
}

void GLCutPlane::setSize(float size)
{
    this->size = size;
}

void GLCutPlane::initialize()
{
    // Save current settings
    GL_SAFE_CALL(glGetBooleanv(GL_DEPTH_TEST, &this->depthTestEnabled));
    GL_SAFE_CALL(glGetBooleanv(GL_LINE_SMOOTH, &this->lineSmoothEnabled));
    GL_SAFE_CALL(glGetIntegerv(GL_LINE_SMOOTH_HINT, &this->lineSmoothHint));
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalizeEnabled));
    GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &this->lightingEnabled));
    GL_SAFE_CALL(glGetFloatv(GL_LINE_WIDTH, &this->lineWidth));
    GL_SAFE_CALL(glGetBooleanv(GL_CULL_FACE, &this->cullFaceEnabled));
    // Initialize environment
    GL_SAFE_CALL(glEnable(GL_DEPTH_TEST));
    GL_SAFE_CALL(glEnable(GL_LINE_SMOOTH));
    GL_SAFE_CALL(glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE));
    GL_SAFE_CALL(glEnable(GL_NORMALIZE));
    GL_SAFE_CALL(glDisable(GL_LIGHTING));
    GL_SAFE_CALL(glDisable(GL_CULL_FACE));
    GL_SAFE_CALL(glLineWidth(1.0f));
}

void GLCutPlane::finalize()
{
    // Restore previous environment
    GL_SAFE_CALL(this->depthTestEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    GL_SAFE_CALL(this->lineSmoothEnabled ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH));
    GL_SAFE_CALL(this->normalizeEnabled ? glEnable(GL_NORMALIZE) :glDisable(GL_NORMALIZE));
    GL_SAFE_CALL(this->lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    GL_SAFE_CALL(this->cullFaceEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
    GL_SAFE_CALL(glHint(GL_LINE_SMOOTH_HINT, this->lineSmoothHint));
}

void GLCutPlane::draw()
{
    GLfloat cutScale = this->getSize();

    GL_SAFE_CALL(glEnable(GL_LIGHTING));
    this->getGLWidget()->qglColor(QColor(255,100,0,255));
    RR3Vector direction(this->normal);
    direction *= 0.5;
    GLArrow glArrow(this->glWidget,this->position,direction,true,true);
    glArrow.paint();
    GL_SAFE_CALL(glDisable(GL_LIGHTING));

    RRMatrix R;

    this->findRotationMatrix(R);

    RNode n1(0.0,-cutScale / 2.0,-cutScale / 2.0);
    RNode n2(0.0, cutScale / 2.0,-cutScale / 2.0);
    RNode n3(0.0, cutScale / 2.0, cutScale / 2.0);
    RNode n4(0.0,-cutScale / 2.0, cutScale / 2.0);
    RNode nw(0.0,-cutScale / 2.0, 0.0           );
    RNode ne(0.0, cutScale / 2.0, 0.0           );
    RNode ns(0.0, 0.0,           -cutScale / 2.0);
    RNode nn(0.0, 0.0,            cutScale / 2.0);

    n1.transform(R,this->position);
    n2.transform(R,this->position);
    n3.transform(R,this->position);
    n4.transform(R,this->position);
    nw.transform(R,this->position);
    ne.transform(R,this->position);
    ns.transform(R,this->position);
    nn.transform(R,this->position);

    this->getGLWidget()->qglColor(QColor(255,255,255,100));

    GL_SAFE_CALL(glNormal3d(this->normal[0],this->normal[1],this->normal[2]));
    GLFunctions::begin(GL_QUADS);
    GL_SAFE_CALL(glVertex3d(n1.getX(),n1.getY(),n1.getZ()));
    GL_SAFE_CALL(glVertex3d(n2.getX(),n2.getY(),n2.getZ()));
    GL_SAFE_CALL(glVertex3d(n3.getX(),n3.getY(),n3.getZ()));
    GL_SAFE_CALL(glVertex3d(n4.getX(),n4.getY(),n4.getZ()));
    GLFunctions::end();

    this->getGLWidget()->qglColor(QColor(255,0,0,255));

    GL_SAFE_CALL(glLineWidth(1.0f));

    GLFunctions::begin(GL_LINE_LOOP);
    GL_SAFE_CALL(glVertex3d(n1.getX(),n1.getY(),n1.getZ()));
    GL_SAFE_CALL(glVertex3d(n2.getX(),n2.getY(),n2.getZ()));
    GL_SAFE_CALL(glVertex3d(n3.getX(),n3.getY(),n3.getZ()));
    GL_SAFE_CALL(glVertex3d(n4.getX(),n4.getY(),n4.getZ()));
    GLFunctions::end();

    GLFunctions::begin(GL_LINES);
    GL_SAFE_CALL(glVertex3d(nw.getX(),nw.getY(),nw.getZ()));
    GL_SAFE_CALL(glVertex3d(ne.getX(),ne.getY(),ne.getZ()));
    GL_SAFE_CALL(glVertex3d(ns.getX(),ns.getY(),ns.getZ()));
    GL_SAFE_CALL(glVertex3d(nn.getX(),nn.getY(),nn.getZ()));
    GLFunctions::end();

    uint nDivs = 10;
    double inc = double(cutScale) / double(nDivs);

    this->getGLWidget()->qglColor(QColor(255,50,0,255));

    GL_SAFE_CALL(glLineWidth(1.0f));

    for (uint i=0;i<=nDivs;i++)
    {
        double zc = i*inc - cutScale / 2.0;

        RNode nodeEast(0.0,-cutScale / 2.0, zc);
        RNode nodeWest(0.0, cutScale / 2.0, zc);

        RNode nodeSouth(0.0, zc,-cutScale / 2.0);
        RNode nodeNorth(0.0, zc, cutScale / 2.0);

        nodeEast.transform(R,this->position);
        nodeWest.transform(R,this->position);
        nodeSouth.transform(R,this->position);
        nodeNorth.transform(R,this->position);

        GLFunctions::begin(GL_LINES);
        GL_SAFE_CALL(glVertex3d(nodeEast.getX(),nodeEast.getY(),nodeEast.getZ()));
        GL_SAFE_CALL(glVertex3d(nodeWest.getX(),nodeWest.getY(),nodeWest.getZ()));
        GL_SAFE_CALL(glVertex3d(nodeSouth.getX(),nodeSouth.getY(),nodeSouth.getZ()));
        GL_SAFE_CALL(glVertex3d(nodeNorth.getX(),nodeNorth.getY(),nodeNorth.getZ()));
        GLFunctions::end();
    }
}
