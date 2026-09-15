#include "gl_functions.h"
#include "gl_cut_plane.h"
#include "gl_arrow.h"
#include "gl_state_cache.h"
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
    this->depthTestEnabled = GLStateCache::instance().getDepthTest();
    this->lineSmoothEnabled = GLStateCache::instance().getLineSmooth();
    this->normalizeEnabled = GLStateCache::instance().getNormalize();
    this->lightingEnabled = GLStateCache::instance().getLighting();
    this->lineWidth = GLStateCache::instance().getLineWidth();
    this->cullFaceEnabled = GLStateCache::instance().getCullFace();
    // Initialize environment
    GLStateCache::instance().setDepthTest(GL_TRUE);
    GLStateCache::instance().setLineSmooth(GL_TRUE);
    GLStateCache::instance().setNormalize(GL_TRUE);
    GLStateCache::instance().disableLighting();
    GLStateCache::instance().setCullFace(GL_FALSE);
    GLStateCache::instance().setLineWidth(1.0f);
}

void GLCutPlane::finalize()
{
    // Restore previous environment
    GLStateCache::instance().setDepthTest(this->depthTestEnabled);
    GLStateCache::instance().setLineSmooth(this->lineSmoothEnabled);
    GLStateCache::instance().setNormalize(this->normalizeEnabled);
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GLStateCache::instance().setCullFace(this->cullFaceEnabled);
    GLStateCache::instance().setLineWidth(this->lineWidth);
}

void GLCutPlane::draw()
{
    GLfloat cutScale = this->getSize();

    GLStateCache::instance().enableLighting();
    this->getGLWidget()->qglColor(QColor(255,100,0,255));
    RR3Vector direction(this->normal);
    direction *= 0.5;
    GLArrow glArrow(this->glWidget,this->position,direction,true,true);
    glArrow.paint();
    GLStateCache::instance().disableLighting();

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

    GLFunctions::normal3d(this->normal[0],this->normal[1],this->normal[2]);
    GLFunctions::begin(GL_TRIANGLE_FAN);
    GLFunctions::vertex3d(n1.getX(),n1.getY(),n1.getZ());
    GLFunctions::vertex3d(n2.getX(),n2.getY(),n2.getZ());
    GLFunctions::vertex3d(n3.getX(),n3.getY(),n3.getZ());
    GLFunctions::vertex3d(n4.getX(),n4.getY(),n4.getZ());
    GLFunctions::end();

    this->getGLWidget()->qglColor(QColor(255,0,0,255));

    GLStateCache::instance().setLineWidth(1.0f);

    GLFunctions::begin(GL_LINE_LOOP);
    GLFunctions::vertex3d(n1.getX(),n1.getY(),n1.getZ());
    GLFunctions::vertex3d(n2.getX(),n2.getY(),n2.getZ());
    GLFunctions::vertex3d(n3.getX(),n3.getY(),n3.getZ());
    GLFunctions::vertex3d(n4.getX(),n4.getY(),n4.getZ());
    GLFunctions::end();

    GLFunctions::begin(GL_LINES);
    GLFunctions::vertex3d(nw.getX(),nw.getY(),nw.getZ());
    GLFunctions::vertex3d(ne.getX(),ne.getY(),ne.getZ());
    GLFunctions::vertex3d(ns.getX(),ns.getY(),ns.getZ());
    GLFunctions::vertex3d(nn.getX(),nn.getY(),nn.getZ());
    GLFunctions::end();

    uint nDivs = 10;
    double inc = double(cutScale) / double(nDivs);

    this->getGLWidget()->qglColor(QColor(255,50,0,255));

    GLStateCache::instance().setLineWidth(1.0f);

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
        GLFunctions::vertex3d(nodeEast.getX(),nodeEast.getY(),nodeEast.getZ());
        GLFunctions::vertex3d(nodeWest.getX(),nodeWest.getY(),nodeWest.getZ());
        GLFunctions::vertex3d(nodeSouth.getX(),nodeSouth.getY(),nodeSouth.getZ());
        GLFunctions::vertex3d(nodeNorth.getX(),nodeNorth.getY(),nodeNorth.getZ());
        GLFunctions::end();
    }
}
