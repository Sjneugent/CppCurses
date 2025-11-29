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
    TorrentExpanderImpl(/* args */);
    ~TorrentExpanderImpl();
    static TorrentExpander Root();
    TorrentExpander Child();

    bool Expand();
    bool Collapse();
    int MinLevel() const;
    int MaxLevel() const;
    bool expanded = false;
    
};


#endif