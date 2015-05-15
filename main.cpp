#include <iostream>
#include <string>
#include <list>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/XmlOutputterHook.h>
#include <cppunit/tools/XmlElement.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/tools/StringTools.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cppunit/TestListener.h>
#include <cppunit/Test.h>
#include <sys/time.h>
#include <sys/wait.h>

// comment it out for real testing
#define TEST_CPPUNIT_DURATION 1

using namespace CppUnit;
using namespace std;

class CBasicMath
{
public:
   int Addition(int x, int y);
   int Multiply(int x, int y);
   int FailureTest();
};

int CBasicMath::Addition(int x, int y)
{
#if TEST_CPPUNIT_DURATION
   usleep(100000);
#endif
   return (x + y);
}

int CBasicMath::Multiply(int x, int y)
{
#if TEST_CPPUNIT_DURATION
   usleep(200000);
#endif
   return (x * y);
}

int CBasicMath::FailureTest()
{
#if TEST_CPPUNIT_DURATION
   usleep(300000);
#endif
   return -1;
}

///////////////////////
/// \brief The TimingListener class
///
class TimingListener : public TestListener
{
private:
    long long current_timestamp(void)
    {
        struct timeval te;
        gettimeofday(&te, NULL); // get current time
        long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
        return milliseconds;
    }

public:
    void startTest( CppUnit::Test *test )
    {
        _chronometer = current_timestamp();
        return;
        // suppress
        test++;
    }

    void endTest( CppUnit::Test *test )
    {
        _chronometer = current_timestamp() - _chronometer;
        std::string testName = test->getName();
        timing_map.insert(std::pair<string, long long>(testName, _chronometer));
    }

    // private interface
    std::map <string, long long> getTimes()
    {
        return timing_map;
    }

private:
    long long _chronometer;
    std::map <string, long long> timing_map;
};

///////////////////////////
/// \brief The MyXmlOutputterHook class
///
class MyXmlOutputterHook : public XmlOutputterHook
 {
private:
    char* getTimeStamp()
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        sprintf(timestr, "%d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        return timestr;
    }

public:
   MyXmlOutputterHook(std::map <string, long long> timing_map)
   {
       this->timing = timing_map;
   }

   virtual ~MyXmlOutputterHook()
   {
   }

   virtual void successfulTestAdded( XmlDocument *document,
                                     XmlElement *testElement,
                                     Test *test )
   {
       // add timing
       std::map<std::basic_string<char>, long long int>::iterator it = timing.find(test->getName());
       long long elapsed = it->second;
       timing.erase(it);
       testElement->addElement( new CppUnit::XmlElement("Time", elapsed));

       return;
       // dead code: to suppress compiler warnings unused parameter
       char* p = (char*) document; p++;
   }

   virtual void failTestAdded( XmlDocument *document,
                               XmlElement *testElement,
                               Test *test,
                               TestFailure *failure )
   {
       // add timing
       std::map<std::basic_string<char>, long long int>::iterator it = timing.find(test->getName());
       long long elapsed = it->second;
       timing.erase(it);
       testElement->addElement( new CppUnit::XmlElement("Time", elapsed));

       return;
       // dead code: to suppress compiler warnings unused parameter
       char* p = (char*) document; p = (char*) failure; p++;
   }

   virtual void statisticsAdded( XmlDocument *document,
                                 XmlElement *statisticsElement )
   {
       statisticsElement->addElement( new CppUnit::XmlElement("Time", 0));
       statisticsElement->addElement( new CppUnit::XmlElement("Timestamp", getTimeStamp()));

       return;
       // dead code: to suppress compiler warnings unused parameter
       char* p = (char*) document; p++;
   }


private:
    std::map <string, long long> timing;
    char timestr[64];
};

//-----------------------------------------------------------------------------
static bool gbAdd = true;
static bool gbMul = true;
static bool gbFai = false;

class TestBasicMath : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestBasicMath);
    if (gbAdd)
        CPPUNIT_TEST(testAddition);
    if (gbMul)
        CPPUNIT_TEST(testMultiply);
    if (gbFai)
        CPPUNIT_TEST(testFailure);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void);

protected:
    void testAddition(void);
    void testMultiply(void);
    void testFailure(void);

private:

    CBasicMath *mTestObj;
};

//-----------------------------------------------------------------------------

void TestBasicMath::testAddition(void)
{
    usleep(1000); // for
    CPPUNIT_ASSERT(5 == mTestObj->Addition(2,3));
}

void TestBasicMath::testMultiply(void)
{
    CPPUNIT_ASSERT(6 == mTestObj->Multiply(2,3));
}

void TestBasicMath::testFailure(void)
{
    CPPUNIT_ASSERT(0 == mTestObj->FailureTest());
}

void TestBasicMath::setUp(void)
{
    mTestObj = new CBasicMath();
}

void TestBasicMath::tearDown(void)
{
    delete mTestObj;
}

//-----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION( TestBasicMath );

int exec_cppunittest(bool add, bool mul, bool fail, int testid)
{
    gbAdd = add;
    gbMul = mul;
    gbFai = fail;

    // informs test-listener about testresults
    CPPUNIT_NS::TestResult testresult;

    // register listener for collecting the test-results
    CPPUNIT_NS::TestResultCollector collectedresults;
    testresult.addListener (&collectedresults);

    // register listener for per-test progress output
    // CPPUNIT_NS::TestListener progress;
    TimingListener progress;
    testresult.addListener(&progress);

    // insert test-suite at test-runner by registry
    CPPUNIT_NS::TestRunner testrunner;
    testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
    testrunner.run(testresult);

    // output results in compiler-format
    CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write ();

    // Output XML for Jenkins CPPunit plugin
    ofstream xmlFileOut("cppRes.xml");
    XmlOutputter xmlOut(&collectedresults, xmlFileOut);
    XmlOutputterHook *hook = (XmlOutputterHook*) new MyXmlOutputterHook(progress.getTimes());
    xmlOut.addHook(hook);
    xmlOut.write();

    int child_pid = fork();
    if (0 == child_pid)
    {
        char resname[64];
        sprintf(resname, "junitResult_%d.xml", testid);
        //$ xsltproc -o junitResults.xml cpp2junit.xslt cppunitResults.xml
        const char *argv[] = {"/usr/bin/xsltproc","-o", resname, "cpp2junit.xslt", "cppRes.xml", NULL};
        execvp(argv[0], (char* const*) argv);
        _exit(0);
    }

    // return 0 if tests were successful
    return child_pid; // collectedresults.wasSuccessful() ? 0 : 1;

}

///////////////////////////////
/// \brief child_wait
/// \param child_pid
///
void child_wait (int child_pid)
{   // wait child exit
    int child_status;
    pid_t tpid = wait(&child_status);
    while(tpid != child_pid)
      tpid = wait(&child_status);
}

////////////////////////////////
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char* argv[])
{
    typedef struct {
        bool a, b, c;
    } testset;
    testset ts[] = {
        {true, true, true},
        {true, true, false},
        {false, true, true},
    };

    for (int i = 0; i < (int) (sizeof(ts)/sizeof(testset)); i++)
    {
        child_wait(exec_cppunittest(ts[i].a, ts[i].b, ts[i].c, i+1));
    }
    return 0;
    // suppress
    argc++, argv++;
}
