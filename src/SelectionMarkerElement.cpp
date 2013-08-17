#include "config.h"
#include "SelectionMarkerElement.h"

#include "RenderObject.h"
#include "RenderImage.h"
#include "Event.h"
#include "Frame.h"
#include "HTMLImageLoader.h"
#include "MouseEvent.h"

#define PIXEL_OFFSET            (1U)

namespace WebCore {

static const String& positionString()
{
    DEFINE_STATIC_LOCAL(String, s, ("position"));
    return s;
}

static const String& zindexString()
{
    DEFINE_STATIC_LOCAL(String, s, ("z-index"));
    return s;
}

static const String& topString()
{
    DEFINE_STATIC_LOCAL(String, s, ("top"));
    return s;
}

static const String& leftString()
{
    DEFINE_STATIC_LOCAL(String, s, ("left"));
    return s;
}

static PassRefPtr<Image> startMarker()
{
    DEFINE_STATIC_LOCAL(RefPtr<Image>, imageStartMarker, (Image::loadPlatformResource("missingImage")));
    return imageStartMarker;
}

static PassRefPtr<Image> endMarker()
{
    DEFINE_STATIC_LOCAL(RefPtr<Image>, imageEndMarker, (Image::loadPlatformResource("searchCancel")));
    return imageEndMarker;
}

class RenderSelectionMarker : public RenderImage {
public:
    RenderSelectionMarker(Node*);

private:
    virtual void layout();
};

RenderSelectionMarker::RenderSelectionMarker(Node* node)
    : RenderImage(node)
{
}

void RenderSelectionMarker::layout()
{
    // Calculate the width and height
    computeLogicalWidth();
    computeLogicalHeight();
    toSelectionMarkerElement(node())->updatePendingPosition();

    RenderImage::layout();
}

RenderObject* SelectionMarkerElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    RenderSelectionMarker* marker = new (arena) RenderSelectionMarker(this);
    marker->setImageResource(RenderImageResource::create());
    return marker;
}

void SelectionMarkerElement::moveTo(const IntPoint& point)
{
    stopDragging();

    m_pendingPosition = point;
    
    if(renderer())
        renderer()->setNeedsLayout(true);
}

void SelectionMarkerElement::updatePendingPosition()
{
    if(IntPoint() != m_pendingPosition) {

        ASSERT(renderer());
        if(MarkerStart == m_type) {
            m_pendingPosition.setX(max(0, m_pendingPosition.x() - renderBox()->width()));
            m_pendingPosition.setY(max(0, m_pendingPosition.y() - renderBox()->height()));
        }

        setPositionFromPoint(m_pendingPosition);
        m_pendingPosition = IntPoint();
    }
}

IntPoint SelectionMarkerElement::position() const
{
    ASSERT(renderer());
    IntPoint point;
    IntPoint location(renderBox()->location());
    if(MarkerStart == m_type) {
        IntSize size(renderBox()->size());
        point.setX(location.x() + size.width() + PIXEL_OFFSET);
        point.setY(location.y() + size.height() + PIXEL_OFFSET);
    } else {
        point.setX(location.x() - PIXEL_OFFSET);
        point.setY(location.y() - PIXEL_OFFSET);
    }
    return point;
}

void SelectionMarkerElement::setPositionFromPoint(const IntPoint& point)
{
    ASSERT(renderer());

    IntPoint currentPosition(renderBox()->x(), renderBox()->y());
    if (point == currentPosition)
        return;

    ExceptionCode ec;
    style()->setProperty(positionString(), String("absolute"), ec);
    style()->setProperty(zindexString(), String("99999"), ec);
    style()->setProperty(leftString(), String::number(point.x()) + String("px"), ec);
    style()->setProperty(topString(), String::number(point.y()) + String("px"), ec);

    renderer()->setNeedsLayout(true);
}

void SelectionMarkerElement::startDragging()
{
    if (Frame* frame = document()->frame()) {
        frame->eventHandler()->setCapturingMouseEventsNode(this);
        m_inDragMode = true;
    }
}

void SelectionMarkerElement::stopDragging()
{
    if (!m_inDragMode)
        return;

    if (Frame* frame = document()->frame())
        frame->eventHandler()->setCapturingMouseEventsNode(0);
    m_inDragMode = false;
    if (renderer())
        renderer()->setNeedsLayout(true);
    
    // Notify the client about marker movement
    ASSERT(m_client);
    m_client->onMarkerMoveEnd();
}

void SelectionMarkerElement::handleMove(const IntPoint& point)
{
    IntPoint position;
    position.setX(max(0, (point.x() - (renderBox()->width() / 2))));
    position.setY(max(0, (point.y() - (renderBox()->height() / 2))));

    setPositionFromPoint(position);

    // Notify the client about marker movement
    ASSERT(m_client);
    m_client->onMarkerMove();
}

void SelectionMarkerElement::defaultEventHandler(Event* event)
{
    if (!event->isMouseEvent()) {
        HTMLDivElement::defaultEventHandler(event);
        return;
    }

    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    bool isLeftButton = mouseEvent->button() == LeftButton;
    const AtomicString& eventType = event->type();

    if (eventType == eventNames().mousedownEvent && isLeftButton) {
        startDragging();
        return;
    } else if (eventType == eventNames().mouseupEvent && isLeftButton) {
        stopDragging();
        return;
    } else if (eventType == eventNames().mousemoveEvent) {
        if (m_inDragMode) {
            handleMove(mouseEvent->absoluteLocation());
        }
        return;
    }

    HTMLDivElement::defaultEventHandler(event);
}

void SelectionMarkerElement::attach()
{
    HTMLDivElement::attach();
    
    if (renderer()) {
        RefPtr<Image> markerImage = m_type == MarkerStart ? startMarker() : endMarker();
        if(!markerImage->isNull())
            toRenderImage(renderer())->imageResource()->setCachedImage(new CachedImage(markerImage.get()));
    }
}

void SelectionMarkerElement::detach()
{
    if (m_inDragMode) {
        if (Frame* frame = document()->frame())
            frame->eventHandler()->setCapturingMouseEventsNode(0);      
    }
    HTMLDivElement::detach();
}

const AtomicString& SelectionMarkerElement::shadowPseudoId() const
{
    DEFINE_STATIC_LOCAL(AtomicString, sliderThumb, ("-webkit-selection-marker"));
    return sliderThumb;
}

}

