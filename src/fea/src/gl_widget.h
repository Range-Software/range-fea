#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <vector>

#include <rbl_local_direction.h>
#include <rbl_plane.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QOpenGLWidget>

#include <QColor>
#include <QMatrix4x4>

#include "gl_display_properties.h"
#include "gl_acion_event.h"
#include "gl_model_list.h"
#include "gl_shader_program.h"
#include "gl_text_renderer.h"
#include "session_entity_id.h"
#include "session_node_id.h"

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

    private:

        //! Local axis viewport width.
        static const GLsizei lAxisWpWidth;
        //! Local axis viewport height.
        static const GLsizei lAxisWpHeight;
        //! Desktop device pixel ratio.
        qreal desktopDevicePixelRatio;

    protected:

        //! Global transformation matrix.
        double gMatrix[16];
        //! Local transformation matrix.
        double lMatrix[16];
        //! model ID.
        uint modelID;
        //! X rotation increment.
        float drx;
        //! Y rotation increment.
        float dry;
        //! X translation increment.
        float dtx;
        //! Y translation increment.
        float dty;
        //! Z translation increment.
        float dtz;
        //! Scale factor increment.
        float dscale;
        //! Model scale factor.
        float mscale;
        //! Scale factor.
        float scale;
        //! Button press start point.
        QPoint bpStart;
        //! Button press end point.
        QPoint bpEnd;
        //! Action event.
        GLActionEvent actionEvent;
        //! OpenGL model list
        GLModelList glModelList;
        //! OpenGL void model list
        GLModelList glVoidModelList;
        //! GLText renderer.
        GLTextRenderer glTextRenderer;
        //! Draw stream line position.
        bool drawStreamLinePosition;
        //! Stream line position.
        RR3Vector streamLinePosition;
        //! Draw geometry scale origin.
        bool drawScaleOrigin;
        //! Geometry scale origin.
        RR3Vector scaleOrigin;
        //! Draw geometry rotation origin.
        bool drawRotationOrigin;
        //! Geometry rotation origin.
        RR3Vector rotationOrigin;
        //! Draw local directions.
        bool drawLocalDirections;
        //! Local directions.
        QList<RLocalDirection> localDirections;
        //! Draw cut plane.
        bool drawCutPlane;
        //! Cut plane.
        RPlane cutPlane;
        //! Draw move nodes.
        bool drawMoveNodes;
        //! List of nodes to be moved.
        QMap<SessionNodeID,RR3Vector> nodesToMove;
        //! Display properties.
        GLDisplayProperties displayProperties;
        //! Use void GL model list.
        bool useGlVoidModelList;
        //! Clipping plane enabled.
        bool clippingPlaneEnabled;
        //! Clipping plane distance.
        double clippingPlaneDistance;
        //! Show rotation sphere.
        bool showRotationSphere;
        //! Use GL_CULL_FACE
        bool useGlCullFace;
        //! Model draw time in milliseconds;
        qint64 modelDrawTime;
        //! Flag indicating lights need to be updated.
        bool lightsNeedUpdate;
        //! Cached number of lights for change detection.
        uint cachedNLights;
        //! Lit geometry shader program (Phong lighting in fragment shader).
        GLShaderProgram mainShaderProgram;
        //! Unlit geometry shader program (flat color / texture passthrough).
        GLShaderProgram flatShaderProgram;
        //! Cached projection matrix (column-major, updated each frame in drawModel).
        double projMatrix[16];
        //! Last color set via qglColor() — used by renderText() instead of glGetDoublev.
        QColor currentGLColor;

    public:

        //! Constructor.
        explicit GLWidget(uint modelID, QWidget *parent = nullptr);

        //! Return assigned modelID.
        uint getModelID() const;

        //! Assign new modelID.
        void setModelID(uint modelID);

        //! Return minimum size of the widget.
        QSize minimumSizeHint() const;

        //! Return recommended size of the widget.
        QSize sizeHint() const;

        //! Return const pointer to OpenGL model list.
        const GLModelList & getGLModelList() const;

        //! Return pointer to OpenGL list.
        GLModelList & getGLModelList();

        //! Return const reference to action event object.
        const GLActionEvent & getGLActionEvent() const;

        //! Return const reference display properties.
        const GLDisplayProperties &getGLDisplayProperties() const;

        //! Return reference display properties.
        GLDisplayProperties &getGLDisplayProperties();

        //! Return whether to use void GL model list.
        bool getUseGLVoidModelList() const;

        //! Set whether to use void GL model list.
        void setUseGLVoidModelList(bool useGlVoidModelList);

        //! Return whether to use GL_CULL_FACE.
        bool getUseGlCullFace() const;

        //! Set whether to use GL_CULL_FACE.
        void setUseGlCullFace(bool useGlCullFace);

        //! Take screenshot.
        void takeScreenShot(const QString &fileName);

        //! Return reference to the main (lit) shader program.
        GLShaderProgram &getMainShaderProgram();

        //! Set OpenGL color.
        void qglColor(const QColor &color);

        //! Set clear color.
        void qglClearColor(const QColor &clearColor);

        //! Render text.
        void renderText(double x,double y,double z,const QString &str,const QFont & font = QFont());

    protected:

        //! Initialize scene.
        void initializeGL();

        //! Resize scene.
        void resizeGL(int width, int height);

        //! Paint scene.
        void paintGL();

        //! Draw background gradient.
        void drawBackgroundGradient();

        //! Draw model.
        void drawModel();

        //! Draw value ranges.
        void drawValueRanges(QPainter &painter);

        //! Draw value ranges.
        void drawValueRange(QPainter &painter,
                            unsigned int rangeCount,
                            double lowerValue,
                            double upperValue,
                            std::vector<RR3Vector> pickedPositions,
                            std::vector<double> pickedValues,
                            const QString &valueRangeFile, const QString &valueRangeName, const QString &valueRangeUnits);

        //! Draw message box.
        void drawMessageBox(QPainter &painter, bool drawBox);

        //! Draw information box.
        void drawInfoBox(QPainter &painter, bool drawBox);

        //! Apply transformations.
        void applyTransformations();

        //! Procress action event.
        void processActionEvent();

        //! Process mouse pressed event.
        void mousePressEvent(QMouseEvent *mouseEvent);

        //! Process mouse pressed event.
        void mouseReleaseEvent(QMouseEvent *mouseEvent);

        //! Process mouse move event.
        void mouseMoveEvent(QMouseEvent *mouseEvent);

        //! Process enter event.
        void enterEvent(QEnterEvent *);

        //! Process leave event.
        void leaveEvent(QEvent *);

        //! Mouse wheel event.
        void wheelEvent(QWheelEvent *mouseEvent);

        //! Process key press event.
        void keyPressEvent(QKeyEvent *keyEvent);

        //! Process key release event.
        void keyReleaseEvent(QKeyEvent *keyEvent);

        //! Calculate model scale.
        void calculateModelScale();

        //! Calulate picking ray.
        void calculatePickRay(const QPoint &screenPosition, double viewDepth, RR3Vector &position, RR3Vector &direction, bool applyModelScale) const;

        //! Calculate screen position from real coordinates.
        void convertModelToScreen(const RR3Vector &realPosition, QPoint &screenPosition) const;

        //! Convert screen coordinates to model coordinates.
        void convertScreenToModel(const QPoint &screenPosition, RR3Vector &modelPosition);

        //! Calculate view depth for glOrtho()
        double calculateViewDepth() const;

        //! Show light.
        void showLight(const RGLLight &rGlLight);

        //! Project global coordinates to widgets local.
        GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
                      const GLdouble model[16], const GLdouble proj[16],
                      const GLint viewport[4],
                      GLdouble * winx, GLdouble * winy, GLdouble * winz);

        //! Transform
        void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);

    public slots:

        //! Reset transformation matrix.
        void resetView(float xRotation, float yRotation, float zRotation);

        //! Set clipping plane enabled.
        void setClippingPlane(bool enabled, double distance);

    private slots:

        //! Slot to catch signal emitted when entity visibility have changed.
        void onEntityVisibilityChanged(uint, REntityGroupType, uint, bool);

        //! Slot to catch signal emitted when display properties have changed.
        void onDisplayPropertiesChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Slot to catch signal emitted when variable data has changed.
        void onVariableDataChanged(uint modelID, RVariableType variableType);

        //! Slot to catch signal emitted when variable data has changed.
        void onVariableDataChanged(const SessionEntityID &entityID, RVariableType);

        //! Slot to catch signal emitted when model has changed
        void onModelChanged(uint modelID);

        //! Slot to catch signal emitted when results have changed
        void onResultsChanged(uint modelID);

        //! Stream line position drawing should begin.
        void onBeginDrawStreamLinePosition(const RR3Vector &streamLinePosition);

        //! Stream line position drawing should end.
        void onEndDrawStreamLinePosition();

        //! Geometry scale origin drawing should begin.
        void onBeginDrawScaleOrigin(const RR3Vector &scaleOrigin);

        //! Geometry scale origin drawing should end.
        void onEndDrawScaleOrigin();

        //! Geometry rotation origin drawing should begin.
        void onBeginDrawRotationOrigin(const RR3Vector &rotationOrigin);

        //! Geometry rotation origin drawing should end.
        void onEndDrawRotationOrigin();

        //! Local directions drawing should begin.
        void onBeginDrawLocalDirections(const QList<RLocalDirection> &localDirections);

        //! Local directions drawing should end.
        void onEndDrawLocalDirections();

        //! Cut plane drawing should begin.
        void onBeginDrawCutPlane(const RPlane &plane);

        //! Cut plane drawing should end.
        void onEndDrawCutPlane();

        //! Move nodes drawing should begin.
        void onBeginDrawMoveNodes(const QMap<SessionNodeID,RR3Vector> &nodesToMove);

        //! Move nodes drawing should end.
        void onEndDrawMoveNodes();

        //! Draw object has beed added.
        void onDrawObjectAdded();

        //! Draw object has beed removed.
        void onDrawObjectRemoved();

        //! Draw object has changed.
        void onDrawObjectChanged(uint);

};

#endif /* GLWIDGET_H */
