#include <iostream>


#include <imebra/imebra.h>
#include <imebra/tcpStream.h>
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

	// Allocate a TCP stream that connects to the DICOM SCP
	imebra::TCPStream tcpStream(TCPActiveAddress("localhost", "5678"));

	// Allocate a stream reader and a writer that use the TCP stream.
	// If you need a more complex stream (e.g. a stream that uses your
	// own services to send and receive data) then use a Pipe
	imebra::StreamReader readSCU(tcpStream.getStreamInput());
	imebra::StreamWriter writeSCU(tcpStream.getStreamOutput());

	// Add all the abstract syntaxes and the supported transfer
	// syntaxes for each abstract syntax (the pair abstract/transfer syntax is
	// called "presentation context")
	imebra::PresentationContext context("1.2.840.10008.5.1.4.1.1.4"); // MR Image Storage
	context.addTransferSyntax("1.2.840.10008.1.2"); // Implicit VR little endian
	imebra::PresentationContexts presentationContexts;
	presentationContexts.addPresentationContext(context);

	// The AssociationSCU constructor will negotiate a connection through
	// the readSCU and writeSCU stream reader and writer
	imebra::AssociationSCU scu("IMEBRA", "CONQUESTSRV1", 1, 1, presentationContexts, readSCU, writeSCU, 0);

	// The DIMSE service will use the negotiated association to send and receive
	// DICOM commands
	imebra::DimseService dimse(scu);

	// Let's prepare a dataset to store on the SCP
	DataSet payload = CodecFactory::load("c:/dicom/samplemr.dcm");
	std::cout << payload.getString(TagId(tagId_t::SOPClassUID_0008_0016),0) << std::endl;


	imebra::CStoreCommand command(
				"1.2.840.10008.5.1.4.1.1.4", //< one of the negotiated abstract syntaxes
				dimse.getNextCommandID(),
				dimseCommandPriority_t::medium,
				payload.getString(TagId(tagId_t::SOPClassUID_0008_0016), 0),
				payload.getString(TagId(tagId_t::SOPInstanceUID_0008_0018), 0),
				"",
				0,
				payload);
	dimse.sendCommandOrResponse(command);
	try
	{
		imebra::DimseResponse response(dimse.getCStoreResponse(command));

		if(response.getStatus() == imebra::dimseStatus_t::success)
		{
			std::cout << "store success" << std::endl;
			// SUCCESS!
		}
		else {
			std::cout << "store failure" << std::endl;
		}
	}
	catch (const StreamEOFError & error)
	{
		// The association has been closed
		std::cout << "stream error: " << error.what() << std::endl;
	}
}