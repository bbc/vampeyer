class VisPlugin
{
  public:
    virtual const char* getName() const = 0;
    virtual void process() = 0;
};
