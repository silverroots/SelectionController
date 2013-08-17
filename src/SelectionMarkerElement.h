#ifndef SelectionMarkerElement_h
#define SelectionMarkerElement_h

#include "HTMLDivElement.h"
#include "IntPoint.h"
#include <wtf/PassRefPtr.h>

#include "HTMLNames.h"

namespace WebCore {

class HTMLImageLoader;
class Event;

class SelectionMarkerClient {
public:
    virtual void onMarkerMove() = 0;
    virtual void onMarkerMoveEnd() = 0;
};

class SelectionMarkerElement : public HTMLDivElement {
public:
    enum MarkerType { MarkerStart = 0, MarkerEnd };
    
    static PassRefPtr<SelectionMarkerElement> create(SelectionMarkerClient*, Document*, MarkerType);

    bool inDragMode() const { return m_inDragMode; }
    void moveTo(const IntPoint&);
    IntPoint position() const;

    void updatePendingPosition();

    virtual void defaultEventHandler(Event*);
    virtual void attach();
    virtual void detach();
    virtual const AtomicString& shadowPseudoId() const;

private:
    SelectionMarkerElement(SelectionMarkerClient*, Document*, MarkerType);
    virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);
    void setPositionFromPoint(const IntPoint&);
    void startDragging();
    void stopDragging();
    void handleMove(const IntPoint&);

    SelectionMarkerClient* m_client;
    bool m_inDragMode;
    MarkerType m_type;
    IntPoint m_pendingPosition;
};

inline SelectionMarkerElement::SelectionMarkerElement(SelectionMarkerClient* client, Document* document, MarkerType type)
    : HTMLDivElement(HTMLNames::divTag, document)
    , m_client(client)
    , m_inDragMode(false)
    , m_type(type)
{
}

inline PassRefPtr<SelectionMarkerElement> SelectionMarkerElement::create(SelectionMarkerClient* client, Document* document, MarkerType type)
{
    return adoptRef(new SelectionMarkerElement(client, document, type));
}

inline SelectionMarkerElement* toSelectionMarkerElement(Node* node)
{
    ASSERT(!node || node->isHTMLElement());
    return static_cast<SelectionMarkerElement*>(node);
}

// This will catch anyone doing an unnecessary cast.
void toSelectionMarkerElement(const SelectionMarkerElement* node);

}

#endif
