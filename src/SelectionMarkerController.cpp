#include "config.h"
#include "SelectionMarkerController.h"

#include "SelectionMarkerElement.h"
#include "Frame.h"
#include "Document.h"
#include "ShadowRoot.h"
#include "HTMLBodyElement.h"
#include "HTMLImageLoader.h"
#include "RenderObject.h"
#include "RenderView.h"
#include "RenderLayer.h"
#include "NodeList.h"
#include "IntPoint.h"
#include "HitTestResult.h"


using namespace WTF;

namespace WebCore {
  
void SelectionMarkerController::update(PassRefPtr<Frame> frame)
{
    RefPtr<Frame> protect(frame);
    DEFINE_STATIC_LOCAL(SelectionMarkerController, controller, (protect));

    controller.updateInternal();
} 

static IntRect offsetForPosition(Position pos)
{
    ASSERT(!pos.isNull());
    InlineBox* inlineBox = NULL;
    int caretOffset = 0;
    pos.getInlineBoxAndOffset(DOWNSTREAM, inlineBox, caretOffset);

    if(0 == caretOffset)
        return IntRect();
    
    RenderObject* renderer = pos.deprecatedNode()->renderer();
    ASSERT(renderer);
    int extraWidthToEndOfLine = 0;
    IntRect caretRect = renderer->localCaretRect(inlineBox, caretOffset, &extraWidthToEndOfLine);
    if (caretRect != IntRect())
        caretRect = renderer->localToAbsoluteQuad(FloatRect(caretRect)).enclosingBoundingBox();

    if(caretRect.x() < 0 || caretRect.y() < 0)
        return IntRect();
    return caretRect;
}

void SelectionMarkerController::updateInternal()
{
    if(m_inController)
        return;
    
    m_inController = true;
    FrameSelection* selection = m_frame->selection();
    ASSERT(selection);

    IntRect selectionBounds = enclosingIntRect(selection->bounds());
    if(selectionBounds.isEmpty()) {
        hideMarkers();
        m_inController = false;
        return;
    }
   
    attachMarkers();

    IntPoint start(selectionBounds.x(), selectionBounds.y());
    IntPoint end(selectionBounds.x() + selectionBounds.width(), selectionBounds.y() + selectionBounds.height());

    IntRect bounds = offsetForPosition(selection->start());
    if(!bounds.isEmpty()) {
        start = bounds.location();
    }
    
    bounds = offsetForPosition(selection->end());
    if(!bounds.isEmpty()) {
        end.setX(bounds.maxX());
        end.setY(bounds.maxY());
    }
    
    showMarkers(start, end);
    m_inController = false;
}

void SelectionMarkerController::createMarkers()
{
    ASSERT(!m_start && !m_end);

    Document* doc = m_frame->document();
    ASSERT(doc);
    m_start = SelectionMarkerElement::create(this, doc, SelectionMarkerElement::MarkerStart);
    m_end = SelectionMarkerElement::create(this, doc, SelectionMarkerElement::MarkerEnd);
}

PassRefPtr<HTMLElement> SelectionMarkerController::parentElement() const
{
    Document* doc = m_frame->document();
    ASSERT(doc);

    // Get the body tag and associate the markers with the tag
    RefPtr<NodeList> nodeList = doc->getElementsByTagNameNS(HTMLNames::bodyTag.namespaceURI(), HTMLNames::bodyTag.localName());
    ASSERT(nodeList.get()->length());
    HTMLBodyElement* body = static_cast<HTMLBodyElement*>(nodeList.get()->item(0));

    return body;
}

void SelectionMarkerController::attachMarkers()
{
    if(NULL == m_start->parentNode()) {
        ASSERT(!m_end->parentNode());

        RefPtr<HTMLElement> parent = parentElement();
        ASSERT(parent);
        
        ExceptionCode ec;
        parent->ensureShadowRoot()->appendChild(m_start, ec);
        parent->ensureShadowRoot()->appendChild(m_end, ec);
    }
}

void SelectionMarkerController::showMarkers(const IntPoint& start, const IntPoint& end)
{
    attachMarkers();
    m_start->moveTo(start);
    m_end->moveTo(end);
}

bool SelectionMarkerController::shouldHide() const
{
    if((m_start && !m_start->inDragMode()) &&
       (m_end && !m_end->inDragMode()))
        return true;
    return false;
}

void SelectionMarkerController::hideMarkers()
{
    if(shouldHide() && m_start->parentNode()) {
        ASSERT(m_end->parentNode());

        RefPtr<HTMLElement> parent = parentElement();
        ASSERT(parent);
        
        ExceptionCode ec;
        parent->ensureShadowRoot()->removeChild(m_start.get(), ec);
        parent->ensureShadowRoot()->removeChild(m_end.get(), ec);
        parent->removeShadowRoot();
    }
}

void SelectionMarkerController::onMarkerMove()
{
    if(m_inController)
        return;
    m_inController = true;
    // Update the selection according to the marker movement
    HitTestRequest request(HitTestRequest::Active);
    HitTestResult resultStart(m_start->position());
    m_frame->contentRenderer()->layer()->hitTest(request, resultStart);
    Node* startNode = resultStart.innerNode();
    VisiblePosition visibleStart;
    if( startNode && startNode->renderer())
        visibleStart = startNode->renderer()->positionForPoint(resultStart.localPoint());
    
    HitTestResult resultEnd(m_end->position());
    m_frame->contentRenderer()->layer()->hitTest(request, resultEnd);
    Node* endNode = resultEnd.innerNode();
    VisiblePosition visibleEnd;
    if( endNode && endNode->renderer())
        visibleEnd = endNode->renderer()->positionForPoint(resultEnd.localPoint());
    
    VisibleSelection newSelection(visibleStart, visibleEnd);
    
    FrameSelection* selection = m_frame->selection();
    ASSERT(selection);
    selection->setSelection(newSelection);
    m_inController = false;
}

void SelectionMarkerController::onMarkerMoveEnd()
{
    updateInternal();
}

}

