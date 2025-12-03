#ifndef TORRENT_EXPANDER_H
#define TORRENT_EXPANDER_H

#include <memory>
#include <vector>

class TorrentExpanderImpl;
using TorrentExpander = std::unique_ptr<TorrentExpanderImpl>;

class TorrentExpanderImpl
{
private:
    /* data */
     void Expand(int minLevel);
     void Collapse(int maxLevel);
     TorrentExpanderImpl * parent_ = nullptr;
     std::vector<TorrentExpanderImpl*> children_;
public:
    ~TorrentExpanderImpl();
    static TorrentExpander Root();
    TorrentExpander Child();

    // Non-recursive controls for per-node expansion.
    void SetExpanded(bool value);
    bool Toggle();

    bool Expand();
    bool Collapse();
    int MinLevel() const;
    int MaxLevel() const;
    bool expanded = false;
    
};


#endif