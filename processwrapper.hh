#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

class ProcessWrapper
{
public:
//  ProcessWrapper();

  static unsigned int findProcess(const char *name, unsigned int pid_skip = 0);
	static unsigned int currentProcessId();
  static bool processExists(unsigned int pid);
};

#endif // PROCESSWRAPPER_H
