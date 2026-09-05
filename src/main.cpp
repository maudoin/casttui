#include "DowncastTUI.h"
#include "DowncastLogic.h"

int main(int argc, char *argv[])
{
    DowncastLogic downcast("castapod.db3");

    FilterButtons::Filter filter = FilterButtons::Filter::All;
    FilterButtons::Callback filterCallback =
        [&](FilterButtons::Filter f)
        {
            std::optional<DowncastLogic::MediaStatus> status;
            switch(f)
            {
                case FilterButtons::Filter::New:
                status = DowncastLogic::MediaStatus::New;
                break;
                case FilterButtons::Filter::Queue:
                status = DowncastLogic::MediaStatus::Queued;
                break;
                case FilterButtons::Filter::Skip:
                status = DowncastLogic::MediaStatus::Skipped;
                break;
                case FilterButtons::Filter::Done:
                status = DowncastLogic::MediaStatus::Done;
                break;
                case FilterButtons::Filter::All:
                status.reset();
                break;
            };
            downcast.setCurrentPodcastRowIndex(std::nullopt, status, DowncastLogic::SetPodcastOption::FORCE_REFRESH);
        };
    DropDown::Callbacks podcastCallbacks
    {
        [&]
        {
            return 1+downcast.podcastCount();
        },
        []()->std::vector<TableWindow::ColProp>
        {
            return {TableWindow::ColProp::colLeft("")};
        },
        [&](int const line)->std::string
        {
            if(line==0)return "All";
            return downcast.podcast(line-1).title;
        },
        [&](int currentItem)
        {
            std::optional<int> newPodcastNumber;
            if(currentItem)newPodcastNumber=currentItem-1;
            downcast.setCurrentPodcastRowIndex(currentItem, std::nullopt, DowncastLogic::SetPodcastOption::FORCE_REFRESH);
        }
    };
    TableWindow::Callbacks showsCallbacks
    {
        [&]
        {
            return downcast.showCount();
        },
        []()->std::vector<TableWindow::ColProp>
        {
            return {
                TableWindow::ColProp::colRight("", 1),
                TableWindow::ColProp::colRight("", 1),
                TableWindow::ColProp::colLeft("Title"),
                TableWindow::ColProp::colRight("Duration", 8),
                TableWindow::ColProp::colLeft("Date", 17)};
        },
        [&](int const col, int const line)->std::string
        {
            int i = filter == FilterButtons::Filter::Done ? line * 2 : line;
            switch(col)
            {
                case 0: return "<";
                case 1: return ">";
                case 2: return downcast.showsInRankRange(line, 1).front().title;
                case 3: return downcast.showsInRankRange(line, 1).front().durationStr();
                case 4: return downcast.showsInRankRange(line, 1).front().dateStr();
            }
            return {};
        }
    };
    return TUIApp::main<Downcast>(argc, argv, podcastCallbacks, showsCallbacks, filterCallback);
}