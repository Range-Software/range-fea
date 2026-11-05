#include <cmath>

#include <QLocale>
#include <QPainter>

#include <rbl_utils.h>

#include "gl_arrow.h"
#include "gl_boundary_conditions.h"
#include "gl_functions.h"
#include "gl_state_cache.h"
#include "gl_widget.h"

//! Maximum number of positions per entity at which symbols are drawn.
static const uint maxSymbolSamples = 200;
//! Maximum number of positions per entity at which text tags are drawn.
static const uint maxTextSamples = 20;
//! Symbol size as a fraction of the view (kept constant on screen).
static const double symbolViewSize = 0.15;
//! Number of segments used to draw cones and circles.
static const uint nRoundSegments = 12;

static RR3Vector offsetVector(const RR3Vector &position, const RR3Vector &direction, double scale)
{
    return RR3Vector(position[0] + direction[0] * scale,
                     position[1] + direction[1] * scale,
                     position[2] + direction[2] * scale);
}

static RR3Vector crossVector(const RR3Vector &v1, const RR3Vector &v2)
{
    RR3Vector v;
    RR3Vector::cross(v1,v2,v);
    return v;
}

GLBoundaryConditions::GLBoundaryConditions(GLWidget *glWidget,
                                           const Model *pModel,
                                           const RElementGroup &elementGroup,
                                           REntityGroupType entityGroupType)
    : GLObject(glWidget)
    , pModel(pModel)
    , pElementGroup(&elementGroup)
    , entityGroupType(entityGroupType)
    , lightingEnabled(GL_TRUE)
    , normalizeEnabled(GL_TRUE)
    , cullFaceEnabled(GL_FALSE)
    , lineWidth(1.0f)
{
}

GLBoundaryConditions::Category GLBoundaryConditions::findCategory(RBoundaryConditionType type)
{
    switch (type)
    {
        case R_BOUNDARY_CONDITION_DISPLACEMENT:
            return FixedSupport;
        case R_BOUNDARY_CONDITION_DISPLACEMENT_NORMAL:
            return NormalSupport;
        case R_BOUNDARY_CONDITION_DISPLACEMENT_ROLLER:
            return RollerSupport;
        case R_BOUNDARY_CONDITION_FORCE:
        case R_BOUNDARY_CONDITION_FORCE_UNIT_AREA:
            return VectorLoad;
        case R_BOUNDARY_CONDITION_PRESSURE:
        case R_BOUNDARY_CONDITION_PRESSURE_EXPLICIT:
        case R_BOUNDARY_CONDITION_PRESSURE_IMPLICIT:
            return NormalLoad;
        case R_BOUNDARY_CONDITION_WEIGHT:
            return WeightLoad;
        case R_BOUNDARY_CONDITION_INFLOW_VELOCITY:
        case R_BOUNDARY_CONDITION_INFLOW_VOLURATE:
            return Inflow;
        case R_BOUNDARY_CONDITION_VELOCITY_NORMAL:
            return NormalVelocity;
        case R_BOUNDARY_CONDITION_WALL:
            return Wall;
        case R_BOUNDARY_CONDITION_WALL_FRICTIONLESS:
            return WallFrictionless;
        case R_BOUNDARY_CONDITION_NONE:
        case R_BOUNDARY_CONDITION_N_TYPES:
            return None;
        default:
            return Scalar;
    }
}

QColor GLBoundaryConditions::getCategoryColor(Category category)
{
    switch (category)
    {
        case FixedSupport:
        case NormalSupport:
        case RollerSupport:
            return QColor(0,170,255);
        case VectorLoad:
        case NormalLoad:
        case WeightLoad:
            return QColor(235,40,40);
        case Inflow:
        case NormalVelocity:
            return QColor(255,150,0);
        case Wall:
        case WallFrictionless:
            return QColor(40,190,90);
        case Scalar:
            return QColor(220,60,220);
        case None:
        default:
            return QColor(128,128,128);
    }
}

QString GLBoundaryConditions::getTag(RBoundaryConditionType type)
{
    switch (type)
    {
        case R_BOUNDARY_CONDITION_ABSORBING_BOUNDARY:     return "Abs";
        case R_BOUNDARY_CONDITION_ACOUSTIC_IMPEDANCE:     return "Z";
        case R_BOUNDARY_CONDITION_CHARGE_DENSITY:         return "rho_e";
        case R_BOUNDARY_CONDITION_CONVECTION_FORCED:      return "h_f";
        case R_BOUNDARY_CONDITION_CONVECTION_NATURAL:     return "h_n";
        case R_BOUNDARY_CONDITION_CONVECTION_SIMPLE:      return "h";
        case R_BOUNDARY_CONDITION_DISPLACEMENT:           return "u";
        case R_BOUNDARY_CONDITION_DISPLACEMENT_NORMAL:    return "u_n";
        case R_BOUNDARY_CONDITION_DISPLACEMENT_ROLLER:    return "u_r";
        case R_BOUNDARY_CONDITION_ELECTRIC_POTENTIAL:     return "V";
        case R_BOUNDARY_CONDITION_FORCE:                  return "F";
        case R_BOUNDARY_CONDITION_FORCE_UNIT_AREA:        return "F/A";
        case R_BOUNDARY_CONDITION_HEAT:                   return "Q";
        case R_BOUNDARY_CONDITION_HEAT_RATE_AREA:         return "q";
        case R_BOUNDARY_CONDITION_HEAT_RATE_VOLUME:       return "q_v";
        case R_BOUNDARY_CONDITION_INFLOW_VELOCITY:        return "v_in";
        case R_BOUNDARY_CONDITION_INFLOW_VOLURATE:        return "Q_in";
        case R_BOUNDARY_CONDITION_PARTICLE_CONCENTRATION: return "c";
        case R_BOUNDARY_CONDITION_PARTICLE_FLUX:          return "j";
        case R_BOUNDARY_CONDITION_PARTICLE_MASS_TRANSFER: return "k_m";
        case R_BOUNDARY_CONDITION_PARTICLE_RATE:          return "c'";
        case R_BOUNDARY_CONDITION_PRESSURE:               return "p";
        case R_BOUNDARY_CONDITION_PRESSURE_EXPLICIT:      return "p";
        case R_BOUNDARY_CONDITION_PRESSURE_IMPLICIT:      return "p";
        case R_BOUNDARY_CONDITION_RADIATION:              return "Rad";
        case R_BOUNDARY_CONDITION_TEMPERATURE:            return "T";
        case R_BOUNDARY_CONDITION_VELOCITY_NORMAL:        return "v_n";
        case R_BOUNDARY_CONDITION_VELOCITY_POTENTIAL:     return "phi";
        case R_BOUNDARY_CONDITION_WALL:                   return "Wall";
        case R_BOUNDARY_CONDITION_WALL_FRICTIONLESS:      return "Wall_f";
        case R_BOUNDARY_CONDITION_WAVE_DISPLACEMENT:      return "u_w";
        case R_BOUNDARY_CONDITION_WEIGHT:                 return "W";
        case R_BOUNDARY_CONDITION_CUSTOM:                 return "BC";
        default:                                          return QString();
    }
}

void GLBoundaryConditions::drawSymbol(QPainter &painter, const QRectF &rect, RBoundaryConditionType type)
{
    const Category category = GLBoundaryConditions::findCategory(type);
    const QColor color = GLBoundaryConditions::getCategoryColor(category);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color,1.5,Qt::SolidLine,Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);

    const double size = std::min(rect.width(),rect.height());
    const QPointF center = rect.center();
    const double half = 0.5*size;

    auto point = [&](double x, double y) { return QPointF(center.x() + x*half,center.y() + y*half); };

    // Cone with apex on the top.
    auto drawCone = [&](double bottom)
    {
        const QPointF cone[3] = { point(0.0,-1.0), point(-0.6,bottom), point(0.6,bottom) };
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPolygon(cone,3);
        painter.restore();
    };

    // Arrow between two points with filled head at the end.
    auto drawArrow = [&](const QPointF &start, const QPointF &end)
    {
        const QPointF axis = end - start;
        const double length = std::hypot(axis.x(),axis.y());
        const QPointF unit = axis/length;
        const QPointF normal(-unit.y(),unit.x());
        const QPointF base = end - 0.8*half*unit;
        const QPointF head[3] = { end, base + 0.4*half*normal, base - 0.4*half*normal };
        painter.drawLine(start,base);
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPolygon(head,3);
        painter.restore();
    };

    switch (category)
    {
        case FixedSupport:
        {
            drawCone(1.0);
            break;
        }
        case NormalSupport:
        {
            drawCone(0.5);
            painter.drawLine(point(-0.9,0.9),point(0.9,0.9));
            break;
        }
        case RollerSupport:
        {
            drawCone(0.3);
            painter.drawEllipse(point(0.0,0.65),0.3*half,0.3*half);
            break;
        }
        case VectorLoad:
        {
            drawArrow(point(-1.0,0.0),point(1.0,0.0));
            break;
        }
        case NormalLoad:
        {
            drawArrow(point(-1.0,0.0),point(0.7,0.0));
            painter.drawLine(point(0.9,-1.0),point(0.9,1.0));
            break;
        }
        case WeightLoad:
        {
            drawArrow(point(0.0,-1.0),point(0.0,1.0));
            break;
        }
        case Inflow:
        {
            drawArrow(point(-1.0,-0.5),point(0.7,-0.5));
            drawArrow(point(-1.0,0.5),point(0.7,0.5));
            painter.drawLine(point(0.9,-1.0),point(0.9,1.0));
            break;
        }
        case NormalVelocity:
        {
            painter.drawLine(point(-0.9,-1.0),point(-0.9,1.0));
            drawArrow(point(-0.7,0.0),point(1.0,0.0));
            break;
        }
        case Wall:
        {
            painter.drawLine(point(-1.0,-0.4),point(1.0,-0.4));
            for (int i=-1;i<=1;i++)
            {
                painter.drawLine(point(0.6*i,-0.4),point(0.6*i-0.5,0.5));
            }
            break;
        }
        case WallFrictionless:
        {
            painter.drawLine(point(-1.0,-0.4),point(1.0,-0.4));
            painter.drawEllipse(point(-0.5,0.2),0.35*half,0.35*half);
            painter.drawEllipse(point(0.5,0.2),0.35*half,0.35*half);
            break;
        }
        case Scalar:
        case None:
        default:
        {
            QFont font(painter.font());
            font.setBold(true);
            painter.setFont(font);
            painter.setPen(color);
            painter.drawText(rect,Qt::AlignCenter,GLBoundaryConditions::getTag(type));
            break;
        }
    }

    painter.restore();
}

void GLBoundaryConditions::initialize()
{
    this->lightingEnabled = GLStateCache::instance().getLighting();
    this->normalizeEnabled = GLStateCache::instance().getNormalize();
    this->cullFaceEnabled = GLStateCache::instance().getCullFace();
    this->lineWidth = GLStateCache::instance().getLineWidth();

    GLStateCache::instance().setNormalize(GL_TRUE);
    GLStateCache::instance().setCullFace(GL_FALSE);
}

void GLBoundaryConditions::finalize()
{
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GLStateCache::instance().setNormalize(this->normalizeEnabled);
    GLStateCache::instance().setCullFace(this->cullFaceEnabled);
    GLStateCache::instance().setLineWidth(this->lineWidth);
}

void GLBoundaryConditions::draw()
{
    if (!this->pModel || this->pElementGroup->getNBoundaryConditions() == 0)
    {
        return;
    }

    RProblemTypeMask problemTypeMask = this->pModel->getProblemTaskTree().getProblemTypeMask();

    std::vector<Sample> samples;
    this->findSamples(maxSymbolSamples,samples);
    if (samples.empty())
    {
        return;
    }

    double size = this->getGLWidget()->findModelLength(symbolViewSize);

    QStringList scalarTexts;

    for (uint i=0;i<this->pElementGroup->getNBoundaryConditions();i++)
    {
        const RBoundaryCondition &bc = this->pElementGroup->getBoundaryCondition(i);

        if (!(RBoundaryCondition::getProblemTypeMask(bc.getType()) & problemTypeMask))
        {
            continue;
        }

        Category category = GLBoundaryConditions::findCategory(bc.getType());
        if (category == None)
        {
            continue;
        }
        if (category == Scalar)
        {
            scalarTexts.append(this->findScalarText(bc));
            continue;
        }

        this->getGLWidget()->qglColor(GLBoundaryConditions::getCategoryColor(category));
        this->drawCondition(bc,samples,size);
    }

    if (!scalarTexts.isEmpty())
    {
        this->getGLWidget()->qglColor(GLBoundaryConditions::getCategoryColor(Scalar));

        QFont font;
        font.setBold(true);

        QString text = scalarTexts.join(", ");
        uint stride = std::max(1u,uint(std::ceil(double(samples.size()) / double(maxTextSamples))));
        for (uint i=0;i<samples.size();i+=stride)
        {
            RR3Vector position(samples[i].position);
            if (samples[i].isNormal)
            {
                position = offsetVector(position,samples[i].direction,0.2*size);
            }
            this->getGLWidget()->renderText(position[0],position[1],position[2],text,font);
        }
    }
}

void GLBoundaryConditions::findSamples(uint maxSamples, std::vector<Sample> &samples) const
{
    samples.clear();

    uint nElements = this->pElementGroup->size();
    if (nElements == 0 || maxSamples == 0)
    {
        return;
    }

    const std::vector<RNode> &nodes = this->pModel->getNodes();

    uint stride = std::max(1u,uint(std::ceil(double(nElements) / double(maxSamples))));
    samples.reserve(std::min(nElements,maxSamples));

    for (uint i=0;i<nElements;i+=stride)
    {
        // Take the first element of the block which is visible (on the model edge).
        uint elementID = RConstants::eod;
        for (uint j=i;j<std::min(i+stride,nElements);j++)
        {
            if (this->pModel->elementIsOnEdge(this->pElementGroup->get(j)))
            {
                elementID = this->pElementGroup->get(j);
                break;
            }
        }
        if (elementID == RConstants::eod)
        {
            continue;
        }

        const RElement &rElement = this->pModel->getElement(elementID);

        Sample sample;
        sample.isNormal = false;
        sample.isTangent = false;
        sample.direction = RR3Vector(0.0,0.0,0.0);
        rElement.findCenter(nodes,sample.position[0],sample.position[1],sample.position[2]);

        switch (RElementGroup::getGroupType(rElement.getType()))
        {
            case R_ENTITY_GROUP_SURFACE:
            {
                if (rElement.findNormal(nodes,sample.direction[0],sample.direction[1],sample.direction[2]))
                {
                    sample.isNormal = (sample.direction.normalize() > RConstants::eps);
                }
                break;
            }
            case R_ENTITY_GROUP_LINE:
            {
                if (rElement.size() >= 2)
                {
                    const RNode &n1 = nodes[rElement.getNodeId(0)];
                    const RNode &n2 = nodes[rElement.getNodeId(1)];
                    sample.direction = RR3Vector(n2.getX() - n1.getX(),
                                                 n2.getY() - n1.getY(),
                                                 n2.getZ() - n1.getZ());
                    sample.isTangent = (sample.direction.normalize() > RConstants::eps);
                }
                break;
            }
            default:
                break;
        }

        samples.push_back(sample);
    }
}

RR3Vector GLBoundaryConditions::findSupportDirection(const RBoundaryCondition &bc, const Sample &sample) const
{
    RR3Vector direction(0.0,0.0,1.0);

    bool useEntered = bc.getHasLocalDirection() &&
                      (bc.getExplicitLocalDirection() || this->entityGroupType == R_ENTITY_GROUP_POINT) &&
                      bc.getLocalDirection().length() > RConstants::eps;

    if (this->entityGroupType == R_ENTITY_GROUP_LINE)
    {
        // Lines are held across the line direction (or across the entered one).
        RR3Vector lineDirection(useEntered ? bc.getLocalDirection() : sample.direction);
        if (lineDirection.length() > RConstants::eps)
        {
            direction = lineDirection.findOrthogonal();
        }
    }
    else if (useEntered)
    {
        direction = bc.getLocalDirection();
    }
    else if (sample.isNormal)
    {
        direction = sample.direction;
    }

    if (direction.normalize() < RConstants::eps)
    {
        direction = RR3Vector(0.0,0.0,1.0);
    }
    return direction;
}

RR3Vector GLBoundaryConditions::findGravityDirection() const
{
    RR3Vector gravity(0.0,0.0,-1.0);

    if (this->pElementGroup->hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_G_ACCELERATION))
    {
        const REnvironmentCondition &ec = this->pElementGroup->getEnvironmentCondition(R_ENVIRONMENT_CONDITION_G_ACCELERATION);
        double time = this->pModel->getTimeSolver().getCurrentTime();

        RR3Vector g(0.0,0.0,0.0);
        const RVariableType components[3] = { R_VARIABLE_G_ACCELERATION_X, R_VARIABLE_G_ACCELERATION_Y, R_VARIABLE_G_ACCELERATION_Z };
        for (uint c=0;c<3;c++)
        {
            uint position = ec.findComponentPosition(components[c]);
            if (position != RConstants::eod)
            {
                g[c] = ec.getComponent(position).get(time);
            }
        }
        if (g.normalize() > RConstants::eps)
        {
            gravity = g;
        }
    }

    return gravity;
}

RR3Vector GLBoundaryConditions::findComponentVector(const RBoundaryCondition &bc,
                                                    RVariableType xType,
                                                    RVariableType yType,
                                                    RVariableType zType) const
{
    double time = this->pModel->getTimeSolver().getCurrentTime();

    RR3Vector vector(0.0,0.0,0.0);
    const RVariableType components[3] = { xType, yType, zType };
    for (uint c=0;c<3;c++)
    {
        uint position = bc.findComponentPosition(components[c]);
        if (position != RConstants::eod)
        {
            vector[c] = bc.getComponent(position).get(time);
        }
    }
    return vector;
}

QString GLBoundaryConditions::findScalarText(const RBoundaryCondition &bc) const
{
    QString text = GLBoundaryConditions::getTag(bc.getType());

    if (bc.size() == 1)
    {
        const RConditionComponent &component = bc.getComponent(0);
        double value = component.get(this->pModel->getTimeSolver().getCurrentTime());
        text += " = " + QLocale().toString(value,'g',4);
        if (!component.getUnits().isEmpty())
        {
            text += " " + component.getUnits();
        }
    }

    return text;
}

void GLBoundaryConditions::drawCondition(const RBoundaryCondition &bc, const std::vector<Sample> &samples, double size) const
{
    double coneHeight = 0.4 * size;
    double coneRadius = 0.15 * size;

    Category category = GLBoundaryConditions::findCategory(bc.getType());

    switch (category)
    {
        case FixedSupport:
        {
            const RVariableType components[3] = { R_VARIABLE_DISPLACEMENT_X, R_VARIABLE_DISPLACEMENT_Y, R_VARIABLE_DISPLACEMENT_Z };
            bool enabled[3] = { false, false, false };
            for (uint c=0;c<3;c++)
            {
                uint position = bc.findComponentPosition(components[c]);
                enabled[c] = (position != RConstants::eod && bc.getComponent(position).getEnabled());
            }
            for (const Sample &sample : samples)
            {
                for (uint c=0;c<3;c++)
                {
                    if (!enabled[c])
                    {
                        continue;
                    }
                    // Place the cone on the outer side of the surface.
                    double side = sample.isNormal ? (sample.direction[c] >= 0.0 ? 1.0 : -1.0) : -1.0;
                    RR3Vector axis(0.0,0.0,0.0);
                    axis[c] = side;
                    this->drawCone(sample.position,axis,coneHeight,coneRadius);
                }
            }
            break;
        }
        case NormalSupport:
        case RollerSupport:
        {
            for (const Sample &sample : samples)
            {
                RR3Vector axis = this->findSupportDirection(bc,sample);
                this->drawCone(sample.position,axis,coneHeight,coneRadius);

                RR3Vector t1 = axis.findOrthogonal();
                t1.normalize();
                RR3Vector t2 = crossVector(axis,t1);
                t2.normalize();

                if (category == NormalSupport)
                {
                    // Ground bar below the cone base.
                    RR3Vector base = offsetVector(sample.position,axis,1.2*coneHeight);
                    this->drawSegment(offsetVector(base,t1,-1.5*coneRadius),offsetVector(base,t1,1.5*coneRadius));
                    this->drawSegment(offsetVector(base,t2,-1.5*coneRadius),offsetVector(base,t2,1.5*coneRadius));
                }
                else
                {
                    // Roller below the cone base.
                    double rollerRadius = 0.5 * coneRadius;
                    RR3Vector center = offsetVector(sample.position,axis,coneHeight + rollerRadius);
                    this->drawCircle(center,t1,rollerRadius);
                    this->drawCircle(center,t2,rollerRadius);
                }
            }
            break;
        }
        case VectorLoad:
        {
            RR3Vector force = (bc.getType() == R_BOUNDARY_CONDITION_FORCE)
                    ? this->findComponentVector(bc,R_VARIABLE_FORCE_X,R_VARIABLE_FORCE_Y,R_VARIABLE_FORCE_Z)
                    : this->findComponentVector(bc,R_VARIABLE_FORCE_UNIT_AREA_X,R_VARIABLE_FORCE_UNIT_AREA_Y,R_VARIABLE_FORCE_UNIT_AREA_Z);
            if (force.normalize() < RConstants::eps)
            {
                break;
            }
            for (const Sample &sample : samples)
            {
                this->drawArrow(sample.position,force,size);
            }
            break;
        }
        case NormalLoad:
        case Inflow:
        case NormalVelocity:
        {
            double value = 1.0;
            if (bc.size() > 0)
            {
                value = bc.getComponent(0).get(this->pModel->getTimeSolver().getCurrentTime());
            }
            for (const Sample &sample : samples)
            {
                if (!sample.isNormal)
                {
                    continue;
                }
                // Surface normals point out of the domain. Pressure pushes on
                // the surface and inflow enters through it, so they act against
                // the normal. Normal velocity acts along the normal. A negative
                // value reverses the direction (suction, outflow).
                RR3Vector direction(sample.direction);
                double sign = (category == NormalVelocity) ? 1.0 : -1.0;
                if (category != Inflow && value < 0.0)
                {
                    sign = -sign;
                }
                direction *= sign;
                this->drawArrow(sample.position,direction,size);
            }
            break;
        }
        case WeightLoad:
        {
            RR3Vector gravity = this->findGravityDirection();
            for (const Sample &sample : samples)
            {
                this->drawArrow(sample.position,gravity,size);
            }
            break;
        }
        case Wall:
        case WallFrictionless:
        {
            double barLength = 0.3 * size;
            for (const Sample &sample : samples)
            {
                RR3Vector normal = sample.isNormal ? sample.direction : this->findSupportDirection(bc,sample);
                RR3Vector tangent = normal.findOrthogonal();
                tangent.normalize();

                // Bar on the surface with hatching on the outer side.
                RR3Vector base = offsetVector(sample.position,normal,0.05*size);
                this->drawSegment(offsetVector(base,tangent,-barLength),offsetVector(base,tangent,barLength));

                if (category == Wall)
                {
                    RR3Vector hatch = offsetVector(normal,tangent,-1.0);
                    hatch.normalize();
                    for (int k=-1;k<=1;k++)
                    {
                        RR3Vector start = offsetVector(base,tangent,k*0.8*barLength);
                        this->drawSegment(start,offsetVector(start,hatch,0.4*barLength));
                    }
                }
                else
                {
                    double rollerRadius = 0.3 * barLength;
                    RR3Vector center = offsetVector(base,normal,rollerRadius);
                    this->drawCircle(offsetVector(center,tangent,-0.5*barLength),crossVector(normal,tangent),rollerRadius);
                    this->drawCircle(offsetVector(center,tangent, 0.5*barLength),crossVector(normal,tangent),rollerRadius);
                }
            }
            break;
        }
        case Scalar:
        case None:
        default:
            break;
    }
}

void GLBoundaryConditions::drawCone(const RR3Vector &apex, const RR3Vector &axis, double height, double radius) const
{
    RR3Vector u = axis.findOrthogonal();
    u.normalize();
    RR3Vector v = crossVector(axis,u);
    v.normalize();

    RR3Vector baseCenter = offsetVector(apex,axis,height);

    std::vector<RR3Vector> rim(nRoundSegments);
    for (uint i=0;i<nRoundSegments;i++)
    {
        double angle = 2.0 * RConstants::pi * double(i) / double(nRoundSegments);
        RR3Vector direction = offsetVector(RR3Vector(u[0]*std::cos(angle),u[1]*std::cos(angle),u[2]*std::cos(angle)),v,std::sin(angle));
        rim[i] = offsetVector(baseCenter,direction,radius);
    }

    GLFunctions::begin(GL_TRIANGLES);
    for (uint i=0;i<nRoundSegments;i++)
    {
        const RR3Vector &r1 = rim[i];
        const RR3Vector &r2 = rim[(i+1)%nRoundSegments];

        // Triangles are wound counter-clockwise when seen from outside, the
        // shader treats clockwise triangles as back faces and flips their normal.

        // Mantle
        RR3Vector e1(r1[0]-apex[0],r1[1]-apex[1],r1[2]-apex[2]);
        RR3Vector e2(r2[0]-apex[0],r2[1]-apex[1],r2[2]-apex[2]);
        RR3Vector normal = crossVector(e2,e1);
        normal.normalize();
        GLObject::glNormalVector(normal);
        GLObject::glVertexVector(apex);
        GLObject::glVertexVector(r2);
        GLObject::glVertexVector(r1);

        // Base
        GLObject::glNormalVector(axis);
        GLObject::glVertexVector(baseCenter);
        GLObject::glVertexVector(r1);
        GLObject::glVertexVector(r2);
    }
    GLFunctions::end();
}

void GLBoundaryConditions::drawCircle(const RR3Vector &center, const RR3Vector &axis, double radius) const
{
    RR3Vector u = axis.findOrthogonal();
    u.normalize();
    RR3Vector v = crossVector(axis,u);
    v.normalize();

    GLStateCache::instance().disableLighting();
    GLStateCache::instance().setLineWidth(2.0f);
    GLFunctions::begin(GL_LINES);
    for (uint i=0;i<nRoundSegments;i++)
    {
        for (uint j=i;j<=i+1;j++)
        {
            double angle = 2.0 * RConstants::pi * double(j) / double(nRoundSegments);
            RR3Vector direction = offsetVector(RR3Vector(u[0]*std::cos(angle),u[1]*std::cos(angle),u[2]*std::cos(angle)),v,std::sin(angle));
            GLObject::glVertexVector(offsetVector(center,direction,radius));
        }
    }
    GLFunctions::end();
    GLStateCache::instance().setLighting(this->lightingEnabled);
}

void GLBoundaryConditions::drawSegment(const RR3Vector &start, const RR3Vector &end) const
{
    GLStateCache::instance().disableLighting();
    GLStateCache::instance().setLineWidth(2.0f);
    GLFunctions::begin(GL_LINES);
    GLObject::glVertexVector(start);
    GLObject::glVertexVector(end);
    GLFunctions::end();
    GLStateCache::instance().setLighting(this->lightingEnabled);
}

void GLBoundaryConditions::drawArrow(const RR3Vector &position, const RR3Vector &direction, double length) const
{
    RR3Vector arrowDirection(direction);
    arrowDirection *= length;
    GLArrow arrow(this->getGLWidget(),position,arrowDirection,true,false,0.25);
    arrow.paint();
}
