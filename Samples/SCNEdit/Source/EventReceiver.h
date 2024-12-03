

using namespace irr;
using namespace gui;

class MyEventReceiver : public IEventReceiver
{
    IrrlichtDevice * device;
    int cnt;

    public:
        virtual bool OnEvent (const SEvent& event);
        bool isStripped();
        bool isHidden();

        MyEventReceiver(IrrlichtDevice * dev);

   private:
      bool ERhandleGUI(const SEvent::SGUIEvent& );
      bool ERhandleMouse(const SEvent::SMouseInput& );
      bool ERhandleKey(const SEvent::SKeyInput& );
      bool ERhandleLogs(const SEvent::SLogEvent& LogEvent);
      IGUIEnvironment* env;

};

