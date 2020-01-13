#include <iostream>


//#include <imebra/imebra.h>
#include <imebra/exceptions.h>
#include <imebra/dimse.h>
#include <imebra/acse.h>
#include <imebra/dataSet.h>
#include <imebra/tcpAddress.h>
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
	imebra::PresentationContext context(uidStudyRootQueryRetrieveInformationModelMOVE_1_2_840_10008_5_1_4_1_2_2_2); // move
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
	imebra::MutableDataSet payload; // We will use the negotiated transfer syntax
	payload.setString(TagId(imebra::tagId_t::QueryRetrieveLevel_0008_0052), "STUDY", imebra::tagVR_t::CS);
	payload.setString(TagId(tagId_t::StudyInstanceUID_0020_000D), "1.3.46.670589.11.0.1.1996082307380006", imebra::tagVR_t::UI);
	const std::string abstractSyntax = uidStudyRootQueryRetrieveInformationModelMOVE_1_2_840_10008_5_1_4_1_2_2_2;

	imebra::CMoveCommand command( abstractSyntax,
		dimse.getNextCommandID(),
		dimseCommandPriority_t::medium,
		uidStudyRootQueryRetrieveInformationModelMOVE_1_2_840_10008_5_1_4_1_2_2_2,
		"IMEBRA",
		payload);
	dimse.sendCommandOrResponse(command);

	try
	{
		imebra::DimseResponse response(dimse.getCMoveResponse(command));
		if (response.getStatus() == imebra::dimseStatus_t::success)
		{
			std::cout << "move success" << std::endl;
			// SUCCESS!
		}
		else {
			std::cout << "move failure" << std::endl;
		}
	}
	catch (const StreamEOFError & error)
	{
		// The association has been closed
		std::cout << "stream error: " << error.what() << std::endl;
	}


}