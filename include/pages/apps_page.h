#ifndef BHBB_APPS_PAGE_H
#define BHBB_APPS_PAGE_H

#include <kernel.h>
#include <paf.h>
#include <vector>
#include <unordered_map>

#include "multi_page_app_list.h"
#include "db.h"
#include "dialog.h"

#define FLAG_ICON_DOWNLOAD       (1)
#define FLAG_ICON_LOAD_SURF      (2)
#define FLAG_ICON_ASSIGN_TEXTURE (4)

namespace apps
{
    class Page : public generic::MultiPageAppList
    {
    public:
        class IconAssignJob : public paf::job::JobItem 
        {
        public:
            
            struct Param {
                SceUInt32 widgetHash;
                paf::string path;

                Param(SceUInt32 targetWidget, paf::string dPath):
                    widgetHash(targetWidget),path(dPath){}
            };

            using paf::job::JobItem::JobItem;

            SceVoid Run();
            SceVoid Finish(){}

            Page *callingPage;
            Param taskParam;

            IconAssignJob(const char *name, Page *caller, Param param):job::JobItem(name),callingPage(caller),taskParam(param){}
        };

        class IconDownloadJob : public paf::job::JobItem 
        {
        public:
            
            struct Param {
                SceUInt32 widgetHash;
                paf::string url, dest;

                Param(SceUInt32 targetWidget, paf::string sUrl, paf::string dPath):
                    widgetHash(targetWidget),url(sUrl),dest(dPath){}
            };

            using paf::job::JobItem::JobItem;

            SceVoid Run();
            SceVoid Finish(){}

            Page *callingPage;
            Param taskParam;

            IconDownloadJob(const char *name, Page *caller, Param param):job::JobItem(name),callingPage(caller),taskParam(param){}
        };

        class IconDownloadThread : public paf::thread::Thread
        {
        private:
            Page *callingPage;
        public:
            paf::thread::Thread::Thread;

            SceVoid EntryFunction();

            IconDownloadThread(Page *callingPage, SceInt32 initPriority = SCE_KERNEL_DEFAULT_PRIORITY_USER, SceSize stackSize = SCE_KERNEL_4KiB, const char *name = "apps::Page::IconDownloadThread"):paf::thread::Thread::Thread(initPriority, stackSize, name),callingPage(callingPage){}
        };

        class IconZipJob : public paf::job::JobItem
        {
        public:
            using paf::job::JobItem::JobItem;

            SceVoid Run();
            SceVoid Finish();
        };

        class LoadJob : public paf::job::JobItem
        {
        public:
            using paf::job::JobItem::JobItem;

            SceVoid Run();
            SceVoid Finish();

            Page *callingPage;

            LoadJob(const char *name, Page *caller):job::JobItem(name),callingPage(caller){}
        };

        class TextureList
        {
        public:
            SceBool Contains(SceUInt64 hash);
            SceVoid Clear(SceBool deleteTextures = SCE_TRUE);
            paf::graph::Surface *Get(SceUInt64 hash);
            SceVoid Add(SceUInt64 hash, paf::graph::Surface *surf);
        private:
            struct Node
            {
                paf::graph::Surface *surf;
                SceUInt64 hash;
                Node(SceUInt64 _hash, paf::graph::Surface *_surf):hash(_hash),surf(_surf){}
            };

            std::vector<Node> list;
        };

        enum PageMode
        {
            PageMode_Browse,
            PageMode_Search
        };

        static SceVoid IconDownloadDecideCB(Dialog::ButtonCode buttonResult, ScePVoid userDat);
        static SceVoid ErrorRetryCB(SceInt32 eventID, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);
        static SceVoid SearchCB(SceInt32 eventID, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);
        static SceVoid CategoryButtonCB(SceInt32 eventID, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);

        //Sets page mode: Search or Browse
        SceVoid SetMode(PageMode mode);
        //Cancels any icon download jobs, Redownloads and parses index and calls Redisplay()
        SceVoid Load();

        //Creates a new page and populates with buttons
        SceVoid PopulatePage(paf::ui::Widget *scrollBox) override;

        //Cancels current icon downloads and assignments, DO NOT CALL FROM MAIN THREAD
        SceVoid CancelIconJobs();

        Page();
        virtual ~Page();
    
    private:


        enum {
            Hash_All = 0x59E75663,
            Hash_Game = 0x6222A81A,
            Hash_Emu = 0xD1A8D19,
            Hash_Port = 0xADC6272A,
            Hash_Util = 0x1EFEFBA6,

            Hash_SearchButton = 0xCCCE2527,
            Hash_SearchEnterButton = 0xAB8CB65E,
            Hash_SearchBackButton = 0x6A2C094C,
        } ButtonHash;

        //Starts adding new icon download *jobs*
        SceVoid StartIconDownloads();
        //Stops adding any new icon download *jobs* to cancel current jobs use CancelIconJobs()
        SceVoid CancelIconDownloads();
        
        SceVoid OnClear() override;
        SceVoid OnCleared() override;

        void OnCategoryChanged(int prev, int curr) override;

        PageMode mode;
        
        db::List appList;
        db::List searchList;
        TextureList loadedTextures;

        IconDownloadThread *iconDownloadThread; //Enqueues downloads
        paf::job::JobQueue *iconDownloadQueue; //Performs downloads
        paf::job::JobQueue *iconAssignQueue; //Performs texture loading and assignments
        SceUID iconFlags;
        std::vector<SceUInt64> textureJobs;
    };
}

#endif