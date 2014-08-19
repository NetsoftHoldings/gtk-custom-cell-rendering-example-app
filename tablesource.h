
#include <string>

namespace NSC {

class NSTableView;

class NSTableViewDelegate {
public:
    virtual int heightOfRow( NSTableView* view, int R) = 0;
};

class NSTableViewDataSource {
public:
    virtual int numberOfColumns( NSTableView* view ) = 0;
    virtual int numberOfRows( NSTableView* view ) = 0;
    virtual std::string valueForColumnAndRow( NSTableView* view, int C, int R ) = 0;
    virtual std::string valueForColumnHeader( NSTableView* view, int C )  = 0;
    virtual bool isGroupRow( NSTableView* view, int R ) = 0;

    virtual int activeRow() = 0;
};

}
