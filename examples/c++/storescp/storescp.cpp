#include <iostream>


// #include <imebra/imebra.h>
#include <imebra/exceptions.h>
#include <imebra/dimse.h>
#include <imebra/acse.h>
#include <imebra/dataSet.h>
#include <imebra/tcpAddress.h>
#include <imebra/tcpStream.h>
#include <imebra/CodecFactory.h>
#include <imebra/tcpListener.h>
#include <imebra/streamReader.h>
#include <imebra/streamWriter.h>
#include <sstream>

#if defined(WIN32) || defined(WIN64)
#include <process.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#endif

#include <memory>
#include <list>

using namespace imebra;

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	imebra::TCPListener tcpListener(TCPPassiveAddress("localhost", "9999"));

	// Wait until a connection arrives or terminate() is called on the tcpListener
	imebra::TCPStream tcpStream(tcpListener.waitForConnection());

	// tcpStream now represents the connected socket. Allocate a stream reader and a writer
	// to read and write on the connected socket
	imebra::StreamReader readSCU(tcpStream.getStreamInput());
	imebra::StreamWriter writeSCU(tcpStream.getStreamOutput());

	// Specify which presentation contexts we accept
	imebra::PresentationContext context("1.2.840.10008.5.1.4.1.1.4");
	context.addTransferSyntax("1.2.840.10008.1.2");
	imebra::PresentationContexts presentationContexts;
	presentationContexts.addPresentationContext(context);

	// The AssociationSCP constructor will negotiate the assocation
	imebra::AssociationSCP scp("", 1, 1, presentationContexts, readSCU, writeSCU, 0, 10);

	// Receive commands via the dimse service
	imebra::DimseService dimse(scp);

	try
	{
		// Receive commands until the association is closed
		for(;;)
		{
			// We assume we are going to receive a C-Store. Normally you should check the command type
			// (using DimseCommand::getCommandType()) and then cast to the proper class.
			imebra::CStoreCommand command(dimse.getCommand().getAsCStoreCommand());

			// The store command has a payload. We can do something with it, or we can
			// use the methods in CStoreCommand to get other data sent by the peer
			imebra::DataSet payload = command.getPayloadDataSet();

			// Do something with the payload
			std::cout << "payload received" << std::endl;

			imebra::CodecFactory::save(payload, "dicomFile.dcm", imebra::codecType_t::dicom);

			// Send a response
			dimse.sendCommandOrResponse(CStoreResponse(command, dimseStatusCode_t::success));
		}
	}
	catch(const StreamEOFError& error)
	{
		// The association has been closed
		std::cout << "stream error: " << error.what() << std::endl;
	}
}