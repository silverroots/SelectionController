#ifndef SelectionMarkerController_h
#define SelectionMarkerController_h

#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>

#include "SelectionMarkerElement.h"

namespace WebCore {

class Frame;
class IntPoint;
  
class SelectionMarkerController : public SelectionMarkerClient {
    WTF_MAKE_NONCOPYABLE(SelectionMarkerController);
public:
    static void update(PassRefPtr<Frame>);
    ~SelectionMarkerController() { }

private:
    SelectionMarkerController(PassRefPtr<Frame> frame)
        : m_frame(frame)
        , m_inController(false)
    {
        createMarkers();
    }
    
    void updateInternal();
    void createMarkers();
    PassRefPtr<HTMLElement> parentElement() const;
    void attachMarkers();
    void showMarkers(const IntPoint&, const IntPoint&);
    bool shouldHide() const;
    void hideMarkers();
    
    // SelectionMarkerClient methods
    virtual void onMarkerMove();
    virtual void onMarkerMoveEnd();
    
    // members
    RefPtr<SelectionMarkerElement> m_start;
    RefPtr<SelectionMarkerElement> m_end;
    RefPtr<Frame> m_frame;
    bool m_inController;
};

}
#endif
