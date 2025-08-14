#include "dicom_send.h"

#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/ofstd/ofconapp.h"    /* for OFConsoleApplication */
#include "dcmtk/ofstd/ofstream.h"    /* for OFStringStream et al. */
#include "dcmtk/dcmdata/dcdict.h"    /* for global data dictionary */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/cmdlnarg.h"  /* for prepareCmdLineArgs */
#include "dcmtk/dcmdata/dcostrmz.h"  /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmnet/dstorscu.h"   /* for DcmStorageSCU */

#include "dcmtk/dcmjpeg/djdecode.h"  /* for JPEG decoders */
#include "dcmtk/dcmjpls/djdecode.h"  /* for JPEG-LS decoders */
#include "dcmtk/dcmdata/dcrledrg.h"  /* for RLE decoder */
#include "dcmtk/dcmjpeg/dipijpeg.h"  /* for dcmimage JPEG plugin */

#ifdef WITH_ZLIB
#include <zlib.h>                    /* for zlibVersion() */
#endif

#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif

/* general definitions */

#define OFFIS_CONSOLE_APPLICATION "dcmsend"

static OFLogger dcmsend_logger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

/* default application entity titles */
#define APPLICATIONTITLE     "Will3D"
#define PEERAPPLICATIONTITLE "ANY-SCP"

/* exit codes for this command line tool */
/* (common codes are defined in "ofexit.h" included from "ofconapp.h") */

// output file errors
#define EXITCODE_CANNOT_WRITE_REPORT_FILE        43

// network errors
#define EXITCODE_CANNOT_INITIALIZE_NETWORK       60
#define EXITCODE_CANNOT_NEGOTIATE_ASSOCIATION    61
#define EXITCODE_CANNOT_SEND_REQUEST             62
#define EXITCODE_CANNOT_ADD_PRESENTATION_CONTEXT 65

/* static helper functions */

// make sure that everything is cleaned up properly
static void cleanup()
{
	// deregister JPEG decoder
	DJDecoderRegistration::cleanup();
	// deregister JPEG-LS decoder
	DJLSDecoderRegistration::cleanup();
	// deregister RLE decoder
	DcmRLEDecoderRegistration::cleanup();
#ifdef DEBUG
	/* useful for debugging with dmalloc */
	dcmDataDict.clear();
#endif
}

DicomSend::DicomSend()
{
}

DicomSend::~DicomSend()
{
}

bool DicomSend::NetworkConnectionCheck(const QString& server_ae_title, const QString& server_ip, const int port)
{
	QByteArray ip = server_ip.toLocal8Bit();
	QByteArray title = server_ae_title.toLocal8Bit();

	const char* opt_peer = ip.data();
	const char* opt_peer_title = title.data();
	const char* opt_our_title = APPLICATIONTITLE;
	OFCmdUnsignedInt opt_port = port;

	OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);
	DcmStorageSCU scu;
	// set AE titles 
	scu.setPeerHostName(opt_peer);
	scu.setPeerPort(OFstatic_cast(Uint16, opt_port));
	scu.setPeerAETitle(opt_peer_title);
	scu.setAETitle(opt_our_title);
	// Use presentation context for FIND/MOVE in study root, propose all uncompressed transfer syntaxes 
	OFList<OFString> ts;
	ts.push_back(UID_LittleEndianExplicitTransferSyntax);
	ts.push_back(UID_BigEndianExplicitTransferSyntax);
	ts.push_back(UID_LittleEndianImplicitTransferSyntax);
	scu.addPresentationContext(UID_FINDStudyRootQueryRetrieveInformationModel, ts);
	scu.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, ts);
	scu.addPresentationContext(UID_VerificationSOPClass, ts);

	/* Initialize network */
	OFCondition result = scu.initNetwork();
	bool is_ok = true;
	if (result.bad())
	{
		//DCMNET_ERROR("Unable to set up the network: " << result.text());
		is_ok = false;
	}

	/* Negotiate Association */
	result = scu.negotiateAssociation();
	if (result.bad())
	{
		//DCMNET_ERROR("Unable to negotiate association: " << result.text());
		is_ok = false;
	}

	/* Let's look whether the server is listening:
	   Assemble and send C-ECHO request
	 */
	result = scu.sendECHORequest(0);
	if (result.bad())
	{
		//DCMNET_ERROR("Could not process C-ECHO with the server: " << result.text());
		is_ok = false;
	}
	scu.releaseAssociation();
	return is_ok;
}

int DicomSend::Do(const QString& server_ip, const int port, const QString& server_ae_title, const QList<QString>& files)
{
#if 1
	//const char* opt_peer = server_ip.toStdString().c_str();
	//const char* opt_peer_title = server_ae_title.toStdString().c_str();

	QByteArray ip = server_ip.toLocal8Bit();
	QByteArray title = server_ae_title.toLocal8Bit();

	const char* opt_peer = ip.data();
	const char* opt_peer_title = title.data();

	const char* opt_our_title = APPLICATIONTITLE;

	E_FileReadMode opt_read_mode = ERM_fileOnly;

	OFCmdUnsignedInt opt_port = port;
	OFCmdUnsignedInt opt_timeout = 0;
	OFCmdUnsignedInt opt_dimse_timeout = 0;
	OFCmdUnsignedInt opt_acse_timeout = 30;
	OFCmdUnsignedInt opt_max_receive_pdu_length = ASC_DEFAULTMAXPDU;
	OFCmdUnsignedInt opt_max_send_pdu_length = 0;
	T_DIMSE_BlockingMode opt_block_mode = DIMSE_BLOCKING;
#ifdef WITH_ZLIB
	OFCmdUnsignedInt opt_compressionLevel = 0;
#endif

	OFBool opt_show_presentation_contexts = OFFalse;
	OFBool opt_halt_on_invalid_file = OFTrue;
	OFBool opt_halt_on_unsuccessful_store = OFTrue;
	OFBool opt_allow_illegal_proposal = OFTrue;
	OFBool opt_check_uid_values = OFTrue;
	OFBool opt_multiple_associations = OFTrue;
	DcmStorageSCU::E_DecompressionMode opt_decompression_mode = DcmStorageSCU::DM_losslessOnly;

	OFBool opt_dicom_dir = OFFalse;
	OFBool opt_scan_dir = OFFalse;
	OFBool opt_recurse = OFFalse;
	const char* opt_scan_pattern = "";
	const char* opt_report_filename = "";

	// register JPEG decoder
	DJDecoderRegistration::registerCodecs();
	// register JPEG-LS decoder
	DJLSDecoderRegistration::registerCodecs();
	// register RLE decoder
	DcmRLEDecoderRegistration::registerCodecs();

	/* print resource identifier */
	OFLOG_DEBUG(dcmsend_logger, rcsid << OFendl);

	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded())
	{
		OFLOG_WARN(dcmsend_logger, "no data dictionary loaded, check environment variable: "
			<< DCM_DICT_ENVIRONMENT_VARIABLE);
	}

	/* start with the real work */
	if (opt_scan_dir)
	{
		OFLOG_INFO(dcmsend_logger, "determining input files ...");
	}
	/* iterate over all input filenames/directories */
	OFList<OFString> input_files;
	OFString param_string;
	for (int i = 0; i < files.size(); i++)
	{
		param_string = OFString(files.at(i).toStdString().c_str());
		/* search directory recursively (if required) */
		if (OFStandard::dirExists(param_string))
		{
			if (opt_scan_dir)
			{
				OFStandard::searchDirectoryRecursively(param_string, input_files, opt_scan_pattern, "" /* dirPrefix */, opt_recurse);
			}
			else
			{
				OFLOG_WARN(dcmsend_logger, "ignoring directory because option --scan-directories is not set: " << param_string);
			}
		}
		else
		{
			input_files.push_back(param_string);
		}
	}

	/* check whether there are any input files at all */
	if (input_files.empty())
	{
		OFLOG_FATAL(dcmsend_logger, "no input files to be processed");
		cleanup();
		return EXITCODE_NO_INPUT_FILES;
	}

	DcmStorageSCU storage_scu;
	OFCondition status;
	unsigned long num_invalid_files = 0;

	/* set parameters used for processing the input files */
	storage_scu.setReadFromDICOMDIRMode(opt_dicom_dir);
	storage_scu.setHaltOnInvalidFileMode(opt_halt_on_invalid_file);

	OFLOG_INFO(dcmsend_logger, "checking input files ...");
	/* iterate over all input filenames */
	const char* current_filename = NULL;
	OFListIterator(OFString) if_iter = input_files.begin();
	OFListIterator(OFString) if_last = input_files.end();
	while (if_iter != if_last)
	{
		current_filename = (*if_iter).c_str();
		/* and add them to the list of instances to be transmitted */
		status = storage_scu.addDicomFile(current_filename, opt_read_mode, opt_check_uid_values);
		if (status.bad())
		{
			/* check for empty filename */
			if (strlen(current_filename) == 0)
			{
				current_filename = "<empty string>";
			}
			/* if something went wrong, we either terminate or ignore the file */
			if (opt_halt_on_invalid_file)
			{
				OFLOG_FATAL(dcmsend_logger, "bad DICOM file: " << current_filename << ": " << status.text());
				cleanup();
				return EXITCODE_INVALID_INPUT_FILE;
			}
			else
			{
				OFLOG_WARN(dcmsend_logger, "bad DICOM file: " << current_filename << ": " << status.text() << ", ignoring file");
			}
			++num_invalid_files;
		}
		++if_iter;
	}

	/* check whether there are any valid input files */
	if (storage_scu.getNumberOfSOPInstances() == 0)
	{
		OFLOG_FATAL(dcmsend_logger, "no valid input files to be processed");
		cleanup();
		return EXITCODE_NO_VALID_INPUT_FILES;
	}
	else
	{
		OFLOG_DEBUG(dcmsend_logger, "in total, there are " << storage_scu.getNumberOfSOPInstances()
			<< " SOP instances to be sent, " << num_invalid_files << " invalid files are ignored");
	}

	/* set network parameters */
	storage_scu.setPeerHostName(opt_peer);
	storage_scu.setPeerPort(OFstatic_cast(Uint16, opt_port));
	storage_scu.setPeerAETitle(opt_peer_title);
	storage_scu.setAETitle(opt_our_title);
	storage_scu.setMaxReceivePDULength(OFstatic_cast(Uint32, opt_max_receive_pdu_length));
	storage_scu.setACSETimeout(OFstatic_cast(Uint32, opt_acse_timeout));
	storage_scu.setDIMSETimeout(OFstatic_cast(Uint32, opt_dimse_timeout));
	storage_scu.setDIMSEBlockingMode(opt_block_mode);
	storage_scu.setVerbosePCMode(opt_show_presentation_contexts);
	storage_scu.setDatasetConversionMode(opt_decompression_mode != DcmStorageSCU::DM_never);
	storage_scu.setDecompressionMode(opt_decompression_mode);
	storage_scu.setHaltOnUnsuccessfulStoreMode(opt_halt_on_unsuccessful_store);
	storage_scu.setAllowIllegalProposalMode(opt_allow_illegal_proposal);

	/* output information on the single/multiple associations setting */
	if (opt_multiple_associations)
	{
		OFLOG_DEBUG(dcmsend_logger, "multiple associations allowed (option --multi-associations used)");
	}
	else
	{
		OFLOG_DEBUG(dcmsend_logger, "only a single associations allowed (option --single-association used)");
	}

	/* add presentation contexts to be negotiated (if there are still any) */
	while ((status = storage_scu.addPresentationContexts()).good())
	{
		if (opt_multiple_associations)
		{
			/* output information on the start of the new association */
			if (dcmsend_logger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
			{
				OFLOG_DEBUG(dcmsend_logger, OFString(65, '-') << OFendl
					<< "starting association #" << (storage_scu.getAssociationCounter() + 1));
			}
			else
			{
				OFLOG_INFO(dcmsend_logger, "starting association #" << (storage_scu.getAssociationCounter() + 1));
			}
		}
		OFLOG_INFO(dcmsend_logger, "initializing network ...");
		/* initialize network */
		status = storage_scu.initNetwork();
		if (status.bad())
		{
			OFLOG_FATAL(dcmsend_logger, "cannot initialize network: " << status.text());
			cleanup();
			return EXITCODE_CANNOT_INITIALIZE_NETWORK;
		}
		OFLOG_INFO(dcmsend_logger, "negotiating network association ...");
		/* negotiate network association with peer */
		status = storage_scu.negotiateAssociation();
		if (status.bad())
		{
			// check whether we can continue with a new association
			if (status == NET_EC_NoAcceptablePresentationContexts)
			{
				OFLOG_ERROR(dcmsend_logger, "cannot negotiate network association: " << status.text());
				// check whether there are any SOP instances to be sent
				const size_t numToBeSent = storage_scu.getNumberOfSOPInstancesToBeSent();
				if (numToBeSent > 0)
				{
					OFLOG_WARN(dcmsend_logger, "trying to continue with a new association "
						<< "in order to send the remaining " << numToBeSent << " SOP instances");
				}
			}
			else
			{
				OFLOG_FATAL(dcmsend_logger, "cannot negotiate network association: " << status.text());
				cleanup();
				return EXITCODE_CANNOT_NEGOTIATE_ASSOCIATION;
			}
		}
		if (status.good())
		{
			OFLOG_INFO(dcmsend_logger, "sending SOP instances ...");
			/* send SOP instances to be transferred */
			status = storage_scu.sendSOPInstances();
			if (status.bad())
			{
				OFLOG_FATAL(dcmsend_logger, "cannot send SOP instance: " << status.text());
				// handle certain error conditions (initiated by the communication peer)
				if (status == DUL_PEERREQUESTEDRELEASE)
				{
					// peer requested release (aborting)
					storage_scu.closeAssociation(DCMSCU_PEER_REQUESTED_RELEASE);
				}
				else if (status == DUL_PEERABORTEDASSOCIATION)
				{
					// peer aborted the association
					storage_scu.closeAssociation(DCMSCU_PEER_ABORTED_ASSOCIATION);
				}
				cleanup();
				return EXITCODE_CANNOT_SEND_REQUEST;
			}
		}
		/* close current network association */
		storage_scu.releaseAssociation();
		/* check whether multiple associations are permitted */
		if (!opt_multiple_associations)
		{
			break;
		}
	}

	/* if anything went wrong, report it to the logger */
	if (status.bad() && (status != NET_EC_NoPresentationContextsDefined))
	{
		OFLOG_ERROR(dcmsend_logger, "cannot add presentation contexts: " << status.text());
		cleanup();
		return EXITCODE_CANNOT_ADD_PRESENTATION_CONTEXT;
	}

	/* create a detailed report on the transfer of instances ... */
	if ((opt_report_filename != NULL) && (strlen(opt_report_filename) > 0))
	{
		/* ... and write it to the specified text file */
		status = storage_scu.createReportFile(opt_report_filename);
		if (status.bad())
		{
			cleanup();
			return EXITCODE_CANNOT_WRITE_REPORT_FILE; // TODO: do we really want to exit?
		}
	}

	/* output some status information on the overall sending process */
	if (dcmsend_logger.isEnabledFor(OFLogger::INFO_LOG_LEVEL))
	{
		OFString summary_text;
		storage_scu.getStatusSummary(summary_text);
		OFLOG_INFO(dcmsend_logger, OFendl << summary_text);
	}

	/* make sure that everything is cleaned up properly */
	cleanup();
	return EXITCODE_NO_ERROR;
#else
	return EXITCODE_NO_ERROR;
#endif
}
