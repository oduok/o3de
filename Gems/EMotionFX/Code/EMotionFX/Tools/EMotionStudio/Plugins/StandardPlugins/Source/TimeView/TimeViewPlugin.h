/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/PlatformIncl.h>
#include <EMotionFX/CommandSystem/Source/MotionEventCommands.h>
#include <EMotionFX/Source/Recorder.h>
#include <Editor/AnimGraphEditorBus.h>
#include <EMotionFX/Tools/EMotionStudio/EMStudioSDK/Source/DockWidgetPlugin.h>
#include <EMotionFX/Tools/EMotionStudio/Plugins/StandardPlugins/Source/TimeView/TimeTrack.h>
#include <EMotionFX/Tools/EMotionStudio/Plugins/StandardPlugins/Source/TimeView/TimeViewShared.h>
#include <QScrollArea>
#endif

namespace EMStudio
{
    // forward declarations
    class TrackDataHeaderWidget;
    class TrackDataHeaderWidget;
    class TrackDataWidget;
    class TrackHeaderWidget;
    class TimeInfoWidget;
    class TimeViewToolBar;
    class MotionWindowPlugin;
    class MotionEventsPlugin;
    class MotionListWindow;
    class MotionEventPresetsWidget;
    class MotionSetsWindowPlugin;

    struct EventSelectionItem
    {
        EMotionFX::MotionEvent* GetMotionEvent();
        EMotionFX::MotionEventTrack* GetEventTrack();

        size_t                          m_eventNr;// the motion event index in its track
        size_t                          m_trackNr;// the corresponding track in which the event is in
        EMotionFX::Motion*              m_motion;// the parent motion of the event track
    };

    class TimeViewPlugin
        : public EMStudio::DockWidgetPlugin
        , private EMotionFX::AnimGraphEditorNotificationBus::Handler
    {
        Q_OBJECT // AUTOMOC
        MCORE_MEMORYOBJECTCATEGORY(TimeViewPlugin, MCore::MCORE_DEFAULT_ALIGNMENT, MEMCATEGORY_STANDARDPLUGINS);
        
        friend class TrackDataHeaderWidget;
        friend class TrackDataWidget;
        friend class TimeViewWidget;
        friend class TimeViewToolBar;
        friend class TimeInfoWidget;
        friend class TrackHeaderWidget;

    public:
        enum
        {
            CLASS_ID = 0x00fab009
        };

        TimeViewPlugin();
        ~TimeViewPlugin();

        // overloaded
        const char* GetName() const override;
        uint32 GetClassID() const override;
        bool GetIsClosable() const override             { return true; }
        bool GetIsFloatable() const override            { return true; }
        bool GetIsVertical() const override             { return false; }

        // overloaded main init function
        bool Init() override;
        EMStudioPlugin* Clone() const override { return new TimeViewPlugin(); }

        void OnBeforeRemovePlugin(uint32 classID) override;

        void ProcessFrame(float timePassedInSeconds) override;

        void SetMode(TimeViewMode mode);
        TimeViewMode GetMode() const { return m_mode; }

        double GetScrollX() const       { return m_scrollX; }

        void DeltaScrollX(double deltaX, bool animate = true);

        void SetScrollX(double scrollX, bool animate = true);

        void CalcTime(double xPixel, double* outPixelTime, uint32* outMinutes, uint32* outSeconds, uint32* outMilSecs, uint32* outFrameNr, bool scaleXPixel = true) const;
        void DecomposeTime(double timeValue, uint32* outMinutes, uint32* outSeconds, uint32* outMilSecs, uint32* outFrameNr) const;
        double PixelToTime(double xPixel, bool isScaledPixel = true) const;
        double TimeToPixel(double timeInSeconds, bool scale = true) const;
        double GetCurrentTime() const;  // returns time in seconds
        void SetCurrentTime(double timeInSeconds);  // set time in seconds, auto updates visuals (redraw)
        void UpdateVisualData();
        bool SnapTime(double* inOutTime, TimeTrackElement* elementToIgnore, double snapThreshold);

        void ZoomToFit();
        void GetDataTimes(double* outMaxTime, double* outClipStart, double* outClipEnd) const;
        double CalcFitScale(double minScale = 1.0, double maxScale = 100.0) const;
        void MakeTimeVisible(double timeValue, double offsetFactor = 0.95, bool animate = true);
        bool GetIsTimeVisible(double timeValue) const;
        float GetTimeScale() const                  { return aznumeric_cast<float>(m_timeScale); }

        void RenderElementTimeHandles(QPainter& painter, uint32 dataWindowHeight, const QPen& pen);
        void DisableAllToolTips();

        void AddTrack(TimeTrack* track);
        void RemoveAllTracks();
        TimeTrack* GetTrack(size_t index)           { return m_tracks[index]; }
        size_t GetNumTracks() const                 { return m_tracks.size(); }
        AZ::Outcome<size_t> FindTrackIndex(const TimeTrack* track) const;
        TimeTrack* FindTrackByElement(TimeTrackElement* element) const;

        void UnselectAllElements();

        double RoundDouble(double x) const;

        TimeTrackElement* GetElementAt(int32 x, int32 y);
        TimeTrack* GetTrackAt(int32 y);

        void UpdateViewSettings();
        void SetScale(double scale, bool animate = true);

        bool FindResizePoint(int32 x, int32 y, TimeTrackElement** outElement, uint32* outID);

        QCursor* GetZoomInCursor() const            { return m_zoomInCursor; }
        QCursor* GetZoomOutCursor() const           { return m_zoomOutCursor; }

        // some getters
        TrackDataHeaderWidget* GetTrackDataHeaderWidget() { return m_trackDataHeaderWidget; }
        TrackDataWidget*    GetTrackDataWidget()    { return m_trackDataWidget; }
        TrackHeaderWidget*  GetTrackHeaderWidget()  { return m_trackHeaderWidget; }
        TimeInfoWidget*     GetTimeInfoWidget()     { return m_timeInfoWidget; }
        TimeViewToolBar*    GetTimeViewToolBar()    { return m_timeViewToolBar; }

        void OnKeyPressEvent(QKeyEvent* event);
        void OnKeyReleaseEvent(QKeyEvent* event);

        void ValidatePluginLinks();
        void UpdateSelection();
        void UpdateMaxHeight();

        void ZoomRect(const QRect& rect);

        size_t GetNumSelectedEvents()                                                   { return m_selectedEvents.size(); }
        EventSelectionItem GetSelectedEvent(size_t index) const                         { return m_selectedEvents[index]; }

        void Select(const AZStd::vector<EventSelectionItem>& selection);

        MCORE_INLINE EMotionFX::Motion* GetMotion() const                               { return m_motion; }
        void SetRedrawFlag();

        uint32 CalcContentHeight() const;

    public slots:
        void ReInit();

    public slots:
        void VisibilityChanged(bool visible);
        void OnSelectionChanged()                   { emit SelectionChanged(); }
        void MotionSelectionChanged();

        void AddMotionEvent(int32 x, int32 y);
        void RemoveMotionEvent(int32 x, int32 y);
        void MotionEventChanged(TimeTrackElement* element, double startTime, double endTime);
        void RemoveSelectedMotionEvents();
        void RemoveAllMotionEvents();
        void OnZoomAll();
        void OnGotoTimeZero();
        void OnResetTimeline();
        void OnCenterOnCurTime();
        void OnShowNodeHistoryNodeInGraph();
        void OnClickNodeHistoryNode();
        void MotionEventTrackChanged(size_t eventNr, float startTime, float endTime, const char* oldTrackName, const char* newTrackName)            { UnselectAllElements(); CommandSystem::CommandHelperMotionEventTrackChanged(eventNr, startTime, endTime, oldTrackName, newTrackName); }
        void OnManualTimeChange(float timeValue);

    signals:
        void SelectionChanged();
        void DeleteKeyPressed();
        void ManualTimeChangeStart(float newTime);  // single click scrub in time line (on mouse down)
        void ManualTimeChange(float newTime);       // scrubbing in the time line (moving with the mouse while mouse button clicked)
        void DoubleClickedRecorderNodeHistoryItem(EMotionFX::Recorder::ActorInstanceData* actorInstanceData, EMotionFX::Recorder::NodeHistoryItem* item);       // double clicked an item
        void ClickedRecorderNodeHistoryItem(EMotionFX::Recorder::ActorInstanceData* actorInstanceData, EMotionFX::Recorder::NodeHistoryItem* item);             // left clicked an item
        void RecorderStateChanged();

    private:
        // AnimGraphEditorNotificationBus
        void OnFocusIn() override;
        void OnShow() override;

        MCORE_DEFINECOMMANDCALLBACK(CommandAdjustMotionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandSelectCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandUnselectCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandClearSelectionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandRecorderClearCallback);
        MCORE_DEFINECOMMANDCALLBACK(UpdateInterfaceCallback);
        AZStd::vector<MCore::Command::Callback*> m_commandCallbacks;

        TrackDataHeaderWidget* m_trackDataHeaderWidget;
        TrackDataWidget*    m_trackDataWidget;
        TrackHeaderWidget*  m_trackHeaderWidget;
        TimeInfoWidget*     m_timeInfoWidget;
        TimeViewToolBar*    m_timeViewToolBar;
        QWidget*            m_mainWidget;

        TimeViewMode m_mode = TimeViewMode::None;
        EMotionFX::Motion*                  m_motion;
        MotionWindowPlugin*                 m_motionWindowPlugin;
        MotionEventsPlugin*                 m_motionEventsPlugin;
        MotionListWindow*                   m_motionListWindow;
        MotionSetsWindowPlugin*             m_motionSetPlugin;
        AZStd::vector<EventSelectionItem>    m_selectedEvents;

        EMotionFX::Recorder::ActorInstanceData* m_actorInstanceData;
        EMotionFX::Recorder::NodeHistoryItem*   m_nodeHistoryItem;
        EMotionFX::Recorder::EventHistoryItem*  m_eventHistoryItem;
        EMotionFX::AnimGraphNode*              m_eventEmitterNode;

        struct MotionInfo
        {
            uint32      m_motionId;
            bool        m_initialized;
            double      m_scale;
            double      m_scrollX;
        };

        MotionInfo* FindMotionInfo(uint32 motionID);
        void UpdateCurrentMotionInfo();

        AZStd::vector<MotionInfo*>   m_motionInfos;
        AZStd::vector<TimeTrack*>    m_tracks;

        double              m_pixelsPerSecond;   // pixels per second
        double              m_scrollX;           // horizontal scroll offset
        double              m_curTime;           // current time
        double              m_fps;               // the frame rate, used to snap time values to and to calculate frame numbers
        double              m_curMouseX;
        double              m_curMouseY;
        double              m_maxTime;           // the end time of the full time bar
        double              m_maxHeight;
        double              m_lastMaxHeight;
        double              m_timeScale;         // the time zoom scale factor
        double              m_maxScale;
        double              m_minScale;
        float               m_totalTime;

        double              m_targetTimeScale;
        double              m_targetScrollX;

        bool                m_isAnimating;
        bool                m_dirty;

        QCursor*            m_zoomInCursor;
        QCursor*            m_zoomOutCursor;

        QPen                m_penCurTimeHandle;
        QPen                m_penTimeHandles;
        QPen                m_penCurTimeHelper;
        QBrush              m_brushCurTimeHandle;
    };
}   // namespace EMStudio
