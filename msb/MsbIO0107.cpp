/**
 * @file MsbIO0107.cpp
 * @brief implements of MsbIO0107 class
 *
 * @author S.Tanaka
 * @date 2007.09.04
 * 
 * Copyright(C) 2006-2014 Eisai Co., Ltd. All rights reserved.
 */

#include "stdafx.h"
#include "MsbIO0107.h"

#include "MsbSample.h"
#include "MsbSampleSet.h"
#include "MsbSpectrum.h"

#include <time.h>
#include <math.h>
#include <zlib.h>


using namespace kome::io::msb;


#include <crtdbg.h>
#ifdef _DEBUG
    #define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )
    #define malloc( s ) _malloc_dbg( s, _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif    // _DEBUG



// static members
MsbManager::DataValue MsbIO0107::m_fileHeader[] = {
	DataValue( TYPE_ULONG,  (unsigned int)0 ),				  //  0: File Header Size
	DataValue( TYPE_UCHAR,  (unsigned char)0 ),				  //  1: Endian
	DataValue( TYPE_USHORT, (unsigned short)1 ),				 //  2: Major Version
	DataValue( TYPE_USHORT, (unsigned short)7 ),				 //  3: Minor Version
	DataValue( TYPE_STRING, (MsbString)0 ),					  //  4: Source File Path
	DataValue( TYPE_STRING, (MsbString)0 ),					  //  5: Instrument
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 //  6: Spectrum Group Align
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 //  7: Spectrum Group Properties Align
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 //  8: Spectrum Data Align
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 //  9: Spectrum Align
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 // 10: Spectrum Part Align
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 // 11: Spectrum Properties Align
	DataValue( TYPE_ULONGLONG,  (unsigned long long)0 ),		 // 12: String Table Align
	DataValue( TYPE_ULONG,  (unsigned int)0 ),				  // 13: Compressed String Table Size
	DataValue( TYPE_ULONG,  (unsigned int)0 ),				  // 14: Uncompressed String Table Size
	DataValue( TYPE_TIME,   (time_t)0 )						  // 15: Time
};

MsbManager::DataValue MsbIO0107::m_dataHeader[] = {
	DataValue( TYPE_ULONG, (unsigned int)0 ),		  //  0: header size
	DataValue( TYPE_ULONG, (unsigned int)0 ),		  //  1: data size
	DataValue( TYPE_ULONG, (unsigned int)0 )		   //  2: number of data
};

MsbManager::DataValue MsbIO0107::m_groupInfo[] = {
	DataValue( TYPE_ULONG,  (unsigned int)0 ),		 //  0: spectrum group ID
	DataValue( TYPE_STRING, (MsbString)0 ),			 //  1: spectrum group name
	DataValue( TYPE_ULONG,  (unsigned int)0 ),		 //  2: parent group ID
	DataValue( TYPE_STRING, (MsbString)0 ),			 //  3: spectrum x axis title
	DataValue( TYPE_STRING, (MsbString)0 ),			 //  4: spectrum y axis title
	DataValue( TYPE_STRING, (MsbString)0 ),			 //  5: chromatogram x axis title
	DataValue( TYPE_STRING, (MsbString)0 )			  //  6: chromatogram y axis title
};

MsbManager::DataValue MsbIO0107::m_spectrumInfo[] = {
	DataValue( TYPE_ULONG,  (unsigned int)0 ),	  //  0: Spectrum ID
	DataValue( TYPE_STRING, (MsbString)0 ),		  //  1: Spectrum Name
	DataValue( TYPE_ULONG,  (unsigned int)0 ),	  //  2: Parent Spectrum
	DataValue( TYPE_ULONG,  (unsigned int)0 ),	  //  3: Spectrum Group
	DataValue( TYPE_USHORT, (unsigned short)0 ),	 //  4: MS Stage
	DataValue( TYPE_FLOAT,  (float)0.0f),			//  5: Precursor
	DataValue( TYPE_FLOAT,  (float)0.0f),			//  6: Start Retention Time
	DataValue( TYPE_FLOAT,  (float)0.0f),			//  7: End Retention Time
	DataValue( TYPE_FLOAT,  (float)0.0f),			//  8: min x
	DataValue( TYPE_FLOAT,  (float)0.0f),			//  9: max x
	DataValue( TYPE_FLOAT,  (float)0.0f),			// 10: tic
	DataValue( TYPE_FLOAT,  (float)0.0f),			// 11: bpc
	DataValue( TYPE_ULONG,  (unsigned int)0 ),	  // 12: Scan Number
	DataValue( TYPE_BOOL,   (bool)false ),		   // 13: Centroided
	DataValue( TYPE_FLOAT,  (float)0.2f ),		   // 14: Resolution
	DataValue( TYPE_CHAR,   (char)0 ),			   // 15: Polarity
	DataValue( TYPE_STRING, (MsbString)0 ),		  // 16: Title
	DataValue( TYPE_BOOL,   (bool)false ),		   // 17: Chromatogram Flag
	DataValue( TYPE_STRING, (MsbString)0 )		   // 18: Icon Name
};

MsbManager::DataValue MsbIO0107::m_partInfo[] = {
	DataValue( TYPE_ULONG, (unsigned int)0 ),				//  0: Spectrum
	DataValue( TYPE_ULONG, (unsigned int)0 ),				//  1: Data Length
	DataValue( TYPE_FLOAT, (float)0.0f ),					 //  2: Start m/z
	DataValue( TYPE_FLOAT, (float)0.0f ),					 //  3: End m/z
	DataValue( TYPE_FLOAT, (float)0.0f ),					 //  4: Total Intensity
	DataValue( TYPE_FLOAT, (float)0.0f ),					 //  5: Min Intensity
	DataValue( TYPE_FLOAT, (float)0.0f ),					 //  6: Max Intensity
	DataValue( TYPE_ULONGLONG, (unsigned long long)0 ),	   //  7: m/z Align
	DataValue( TYPE_ULONGLONG, (unsigned long long)0 )		//  8: Intensity Align
};

MsbManager::DataValue MsbIO0107::m_prop[] = {
	DataValue( TYPE_ULONG,  (unsigned int)0 ),	  //  0: Spectrum
	DataValue( TYPE_STRING, (MsbString)0 ),		  //  1: Property Key
	DataValue( TYPE_STRING, (MsbString)0 )		   //  2: Property Value
};

// constructor
MsbIO0107::MsbIO0107() {
}

// destructor
MsbIO0107::~MsbIO0107() {
}

// create string table
void MsbIO0107::createStringTable( char** stringTable ) {
	// get size
	unsigned long stringTableSize = m_fileHeader[ 14 ].val.ul;
	if( stringTableSize == 0 ) {
		return;
	}

	// keep position
	unsigned long long pos = filetell( m_fp );

	// create string table array
	char* st = new char[ stringTableSize ];
	*stringTable = st;

	// create buff
	unsigned int buffSize = m_fileHeader[ 13 ].val.ul;
	unsigned char* buff = new unsigned char[ buffSize ];

	// read string table
	fileseek( m_fp, m_fileHeader[ 12 ].val.ull, SEEK_SET );
	fread( buff, 1, buffSize, m_fp );

	// uncompress
	uncompress( (Bytef*)st, &stringTableSize, buff, buffSize );

	// delete buffer
	delete[] buff;

	// restore position
	fileseek( m_fp, pos, SEEK_SET );
}

// get file header info
void MsbIO0107::getFileHeaderInfo( MsbSample& sample, char* stringTable ) {
	// add file info
	sample.getMsbSampleSet()->setSourceFilePath( stringTable + m_fileHeader[ 4 ].val.ul );
	sample.setInstrument( stringTable + m_fileHeader[ 5 ].val.ul );
}

// read spectrum group
void MsbIO0107::readDataGroupNodes(
		kome::objects::DataGroupNode* parentGroup,
		std::map< unsigned int, kome::objects::DataGroupNode* >& groupMap,
		char* stringTable 
) {
	// read header
	READ_FROM_FILE( m_dataHeader );

	// check map
	if( groupMap.find( 0 ) == groupMap.end() ) {
		groupMap[ 0 ] = parentGroup;
	}

	// get size
	unsigned int size   = m_dataHeader[ 1 ].val.ul;
	unsigned int length = m_dataHeader[ 2 ].val.ul;
	if( size == 0 || length == 0 ) {
		return;
	}

	// create buffer
	unsigned char* buff = new unsigned char[ size * length ];

	// read
	fread( buff, size, length, m_fp );

	// get spectrum group
	unsigned char* pos = buff;
	for( unsigned int i = 0; i < length; i++ ) {
		// get data from buffer
		READ_FROM_BUFFER( m_groupInfo, pos );

		// parent group
		kome::objects::DataGroupNode* parent = groupMap[ m_groupInfo[ 2 ].val.ul ];

		// create group
		kome::objects::DataGroupNode* group
			= parent->createChildGroup( stringTable + m_groupInfo[ 1 ].val.ms );

		// set axis title
		group->setSpecXTitle( stringTable + m_groupInfo[ 3 ].val.ms );
		group->setSpecYTitle( stringTable + m_groupInfo[ 4 ].val.ms );
		group->setChromXTitle( stringTable + m_groupInfo[ 5 ].val.ms );
		group->setChromYTitle( stringTable + m_groupInfo[ 6 ].val.ms );

		// set to map
		groupMap[ m_groupInfo[ 0 ].val.ul ] = group;

		// move read position
		pos += size;
	}

	// delete buffer
	delete[] buff;
}

// read spectrum group properties
void MsbIO0107::readDataGroupNodeProperties(
		std::map< unsigned int, kome::objects::DataGroupNode* >& groupMap,
		char* stringTable
) {
   // read header
	READ_FROM_FILE( m_dataHeader );

	// get size
	unsigned int size   = m_dataHeader[ 1 ].val.ul;
	unsigned int length = m_dataHeader[ 2 ].val.ul;
	if( size == 0 || length == 0 ) {
		return;
	}

	// create buffer
	unsigned char* buff = new unsigned char[ size * length ];

	// read
	fread( buff, size, length, m_fp );

	// get spectrra
	unsigned char* pos = buff;
	for( unsigned int i = 0; i < length; i++ ) {
		// get data from buffer
		READ_FROM_BUFFER( m_prop, pos );

		// get spectrum
		kome::objects::DataGroupNode* group = groupMap[ m_prop[ 0 ].val.ul ];

		// add property
		group->getProperties().setValue(
			stringTable + m_prop[ 1 ].val.ms,
			stringTable + m_prop[ 2 ].val.ms
		);

		// move read position
		pos += size;
	}

	// delete buffer
	delete[] buff;
}

// read spectra
void MsbIO0107::readSpectra(
		MsbSample& sample,
		std::map< unsigned int, MsbSpectrum* >& specMap,
		std::map< unsigned int, kome::objects::DataGroupNode* >& groupMap,
		char* stringTable
) {
	// read header
	READ_FROM_FILE( m_dataHeader );

	// check map
	if( specMap.find( 0 ) == specMap.end() ) {
		specMap[ 0 ] = NULL;
	}

	// get size
	unsigned int size   = m_dataHeader[ 1 ].val.ul;
	unsigned int length = m_dataHeader[ 2 ].val.ul;
	if( size == 0 || length == 0 ) {
		return;
	}

	// create buffer
	unsigned char* buff = new unsigned char[ size * length ];

	// read
	fread( buff, size, length, m_fp );

	// get spectrra
	unsigned char* pos = buff;
	for( unsigned int i = 0; i < length; i++ ) {
		// get data from buffer
		READ_FROM_BUFFER( m_spectrumInfo, pos );

		// create spectrum
		MsbSpectrum* spec = new MsbSpectrum(
			&sample,
			stringTable + m_spectrumInfo[ 1 ].val.ms,
			m_fp
		);

		// add to group
		groupMap[ m_spectrumInfo[ 3 ].val.ul ]->addSpectrum( spec );

		// ID
		specMap[ m_spectrumInfo[ 0 ].val.ul ] = spec;

		// set info
		spec->setParentSpectrum( specMap[ m_spectrumInfo[ 2 ].val.ul ] );
		spec->setMsStage( m_spectrumInfo[ 4 ].val.us );
		spec->setPrecursor( (double)m_spectrumInfo[ 5 ].val.f );
		spec->setRt(
			(double)m_spectrumInfo[ 6 ].val.f,
			(double)m_spectrumInfo[ 7 ].val.f
		);

		spec->setXRange( (double)m_spectrumInfo[ 8 ].val.f, (double)m_spectrumInfo[ 9 ].val.f );
		spec->setTotalIntensity( (double)m_spectrumInfo[ 10 ].val.f );
		spec->setMaxIntensity( (double)m_spectrumInfo[ 11 ].val.f );

		spec->setScanNumber( m_spectrumInfo[ 12 ].val.ul );
		spec->setCentroidMode( m_spectrumInfo[ 13 ].val.b );
		spec->setResolution( m_spectrumInfo[ 14 ].val.f );

		char polarity = m_spectrumInfo[ 15 ].val.c;
		if( polarity < 0 ) {
			spec->setPolarity( kome::objects::Spectrum::POLARITY_NEGATIVE );
		}
		else if( polarity > 0 ) {
			spec->setPolarity( kome::objects::Spectrum::POLARITY_POSITIVE );
		}
		else {
			spec->setPolarity( kome::objects::Spectrum::POLARITY_UNKNOWN );
		}

		spec->setTitle( stringTable + m_spectrumInfo[ 16 ].val.ms );
		spec->setHasChromatogram( m_spectrumInfo[ 17 ].val.b );
		spec->setIcon( stringTable + m_spectrumInfo[ 18 ].val.ms );

		// move read position
		pos += size;
	}

	// delete buffer
	delete[] buff;
}

// read spectrum part
void MsbIO0107::readSpectrumParts(
		std::map< unsigned int, MsbSpectrum* >& specMap
){
	// read header
	READ_FROM_FILE( m_dataHeader );

	// get size
	unsigned int size   = m_dataHeader[ 1 ].val.ul;
	unsigned int length = m_dataHeader[ 2 ].val.ul;
	if( size == 0 || length == 0 ) {
		return;
	}

	// create buffer
	unsigned char* buff = new unsigned char[ size * length ];

	// read
	fread( buff, size, length, m_fp );

	// get spectrra
	unsigned char* pos = buff;
	for( unsigned int i = 0; i < length; i++ ) {
		// get data from buffer
		READ_FROM_BUFFER( m_partInfo, pos );

		// get spectrum
		MsbSpectrum* spec = specMap[ m_partInfo[ 0 ].val.ul ];

		// add part
		spec->addPart(
			m_partInfo[ 1 ].val.ul,
			m_partInfo[ 2 ].val.f,
			m_partInfo[ 3 ].val.f,
			m_partInfo[ 4 ].val.f,
			m_partInfo[ 5 ].val.f,
			m_partInfo[ 6 ].val.f,
			m_partInfo[ 7 ].val.ull,
			m_partInfo[ 8 ].val.ull
		);

		// move read position
		pos += size;
	}

	// delete buffer
	delete[] buff;
}

// read spectrum property
void MsbIO0107::readSpectrumProperties(
		std::map< unsigned int, MsbSpectrum* >& specMap,
		char* stringTable
		
){
	// read header
	READ_FROM_FILE( m_dataHeader );

	// get size
	unsigned int size   = m_dataHeader[ 1 ].val.ul;
	unsigned int length = m_dataHeader[ 2 ].val.ul;
	if( size == 0 || length == 0 ) {
		return;
	}

	// create buffer
	unsigned char* buff = new unsigned char[ size * length ];

	// read
	fread( buff, size, length, m_fp );

	// get spectrra
	unsigned char* pos = buff;
	for( unsigned int i = 0; i < length; i++ ) {
		// get data from buffer
		READ_FROM_BUFFER( m_prop, pos );

		// get spectrum
		MsbSpectrum* spec = specMap[ m_prop[ 0 ].val.ul ];

		// add property
		spec->getProperties().setValue(
			stringTable + m_prop[ 1 ].val.ms,
			stringTable + m_prop[ 2 ].val.ms
		);

		// move read position
		pos += size;
	}

	// delete buffer
	delete[] buff;
}

// write spectrum groups
void MsbIO0107::writeDataGroupNodes(
		std::vector< kome::objects::DataGroupNode* >& groups,
		std::map< kome::objects::DataGroupNode*, unsigned int >& groupMap
) {
	// spectrum group data size
	unsigned int dataSize = GET_DATA_SIZE( m_groupInfo );

	// header
	m_dataHeader[ 0 ].val.ul = GET_DATA_SIZE( m_dataHeader );	// header size
	m_dataHeader[ 1 ].val.ul = dataSize;						 // spectrum group size
	m_dataHeader[ 2 ].val.ul = groups.size();					// number of spectrum groups

	// write header
	WRITE_TO_FILE( m_dataHeader );
	if( groups.size() == 0 ) {
		return;
	}

	// buffer
	unsigned int size = dataSize * groups.size();
	unsigned char* buff = new unsigned char[ size ];
	unsigned char* pos = buff;

	// write group data
	for( unsigned int i = 0; i < groups.size(); i++ ) {
		// group
		kome::objects::DataGroupNode* group = groups[ i ];

		// ID		
		unsigned id =  i + 1;
		groupMap[ group ] = id;
		m_groupInfo[ 0 ].val.ul = id;						  // Spectrum Group ID

		// name
		m_groupInfo[ 1 ].val.ms = st( group->getName() );	  // Spectrum Group Name

		// parent
		unsigned int parent = 0;
		if( group->getParentGroup() != NULL ) {
			parent = groupMap[ group->getParentGroup() ];
		}
		m_groupInfo[ 2 ].val.ul = parent;					  // Parent Spectrum Group

		// axis title
		m_groupInfo[ 3 ].val.ms = st( group->getSpecXTitle() );
		m_groupInfo[ 4 ].val.ms = st( group->getSpecYTitle() );
		m_groupInfo[ 5 ].val.ms = st( group->getChromXTitle() );
		m_groupInfo[ 6 ].val.ms = st( group->getChromYTitle() );

		// write to buffer
		WRITE_TO_BUFFER( m_groupInfo, pos );
		pos += dataSize;
	}

	// write to file
	fwrite( buff, 1, size, m_fp );

	// delete buffer
	delete[] buff;
}

// write spectrum group properties
void MsbIO0107::writeDataGroupNodeProperties(
		std::vector< kome::objects::DataGroupNode* >& groups,
		std::map< kome::objects::DataGroupNode*, unsigned int >& groupMap
) {
	// get properties
	std::vector< Prop > properties;

	for( unsigned i = 0; i < groups.size(); i++ ) {
		// spectrum group
		kome::objects::DataGroupNode* group = groups[ i ];
		unsigned int id = groupMap[ group ];

		// property
		kome::core::Properties& props = group->getProperties();

		// add to array
		unsigned int idx = properties.size();
		unsigned int num = props.getNumberOfProperties();
		if( num > 0 ) {
			// resize
			properties.resize( idx + num );

			// set value
			for( unsigned int j = 0; j < num; j++ ) {
				Prop& prop = properties[ idx + j ];
				prop.id = id;
				prop.key = st( props.getKey( j ) );
				prop.value = st( props.getValue( j ) );
			}
		}
	}

	// spectrum property data size
	unsigned int dataSize = GET_DATA_SIZE( m_prop );

	// header
	m_dataHeader[ 0 ].val.ul = GET_DATA_SIZE( m_dataHeader );	// header size
	m_dataHeader[ 1 ].val.ul = dataSize;						 // spectrum property size
	m_dataHeader[ 2 ].val.ul = properties.size();				// number of spectrum properties

	// write header
	WRITE_TO_FILE( m_dataHeader );
	if( properties.size() == 0 ) {
		return;
	}

	// buffer
	unsigned int size = dataSize * properties.size();
	unsigned char* buff = new unsigned char[ size ];
	unsigned char* pos = buff;

	// write to buffer
	for( unsigned int i = 0; i < properties.size(); i++ ) {
		// property
		Prop& prop = properties[ i ];

		// set data
		m_prop[ 0 ].val.ul = prop.id;	   // Spectrum ID
		m_prop[ 1 ].val.ms = prop.key;	  // Property Key
		m_prop[ 2 ].val.ms = prop.value;	// Property Value

		// write to header
		WRITE_TO_BUFFER( m_prop, pos );
		pos += dataSize;
	}

	// write to file
	fwrite( buff, 1, size, m_fp );

	// delete buffer
	delete[] buff;

}

// write spectrum data
void MsbIO0107::writeSpectrumData(
		unsigned int id,
		kome::objects::Spectrum& spec,
		kome::core::DataPoints& points,
		std::vector< SpectrumPartInfo >& parts
) {
	// number of points
	unsigned int length = points.getLength();
	if( length == 0 ) {
		return;
	}

	// number of parts
	SpectrumPartArray arr;
	getParts( points, arr );
	unsigned int num = arr.getNumberOfParts();

	// save data
	unsigned long long xPos = filetell( m_fp );
	fwrite( points.getXData(), sizeof( float ), length, m_fp );

	unsigned long long yPos = filetell( m_fp );
	fwrite( points.getYData(), sizeof( float ), length, m_fp );

	// add parts
	float maxMz = float();
	SpectrumPartInfo* lastPart = NULL;
	int startIdx = 0;
	double partRange = (double)m_partRange;

	for( unsigned int i = 0; i < points.getLength(); i++ ) {
		float x = (float)points.getX( i );
		float y = (float)points.getY( i );

		if( x > maxMz || lastPart == NULL ) {		// create new parts
			int num = roundnum( x / partRange );
			float mz = (float)num * partRange;
			maxMz = mz + partRange / 2.0f;

			parts.resize( parts.size() + 1 );
			lastPart = &parts.back();

			lastPart->spectrum = id;
			lastPart->length   = 1;
			lastPart->start = x;
			lastPart->end = x;
			lastPart->totalIntensity = y;
			lastPart->minIntensity = y;
			lastPart->maxIntensity = y;
			lastPart->msAlign = xPos + i * sizeof( float );
			lastPart->intAlign = yPos + i * sizeof( float );
		}
		else {
			float totalIntensity = lastPart->totalIntensity + y;
			float minIntensity = MIN( lastPart->minIntensity, y );
			float maxIntensity = MAX( lastPart->maxIntensity, y );
			unsigned int length = lastPart->length + 1;

			lastPart->end = x;
			lastPart->totalIntensity = totalIntensity;
			lastPart->minIntensity = minIntensity;
			lastPart->maxIntensity = maxIntensity;
			lastPart->length = length;
		}
	}
}

// write spectra
void MsbIO0107::writeSpectra(
		kome::objects::DataSet& dataSet,
		std::map< kome::objects::Spectrum*, unsigned int >& spectrumMap,
		std::map< kome::objects::DataGroupNode*, unsigned int >& groupMap
) {
	// spectrum data size
	unsigned int dataSize = GET_DATA_SIZE( m_spectrumInfo );

	// header
	m_dataHeader[ 0 ].val.ul = GET_DATA_SIZE( m_dataHeader );	// header size
	m_dataHeader[ 1 ].val.ul = dataSize;						 // spectrum size
	m_dataHeader[ 2 ].val.ul = dataSet.getNumberOfSpectra();	 // number of spectra

	// write header
	WRITE_TO_FILE( m_dataHeader );
	if( dataSet.getNumberOfSpectra() == 0 ) {
		return;
	}

	// buffer
	unsigned int size = dataSize * dataSet.getNumberOfSpectra();
	unsigned char* buff = new unsigned char[ size ];
	unsigned char* pos = buff;

	// write to buffer
	for( unsigned int i = 0; i < dataSet.getNumberOfSpectra(); i++ ) {
		// spectrum
		kome::objects::Spectrum* spec = dataSet.getSpectrum( i );

		// set data
		m_spectrumInfo[ 0 ].val.ul = spectrumMap[ spec ];					   // Spectrum ID
		m_spectrumInfo[ 1 ].val.ms = st( spec->getName() );					 // Spectrum Name

		int parent = 0;
		if( spec->getParentSpectrum() != NULL ) {
			parent = spectrumMap[ spec->getParentSpectrum() ];
		}
		m_spectrumInfo[ 2 ].val.ul = parent;									// Parent Spectrum

		m_spectrumInfo[ 3 ].val.ul   = groupMap[ spec->getGroup() ];	// Spectrum Group
		m_spectrumInfo[ 4 ].val.us   = spec->getMsStage();					  // MS Stage
		m_spectrumInfo[ 5 ].val.f	= spec->getPrecursor();					// Precursor
		m_spectrumInfo[ 6 ].val.f	= spec->getStartRt();					  // Start Retention Time
		m_spectrumInfo[ 7 ].val.f	= spec->getEndRt();						// End Retention Time
		m_spectrumInfo[ 8 ].val.f	= spec->getMinX();						 // min x
		m_spectrumInfo[ 9 ].val.f	= spec->getMaxX();						 // max x
		m_spectrumInfo[ 10 ].val.f   = spec->getTotalIntensity();			   // total intensity
		m_spectrumInfo[ 11 ].val.f   = spec->getMaxIntensity();				 // max intensity
		m_spectrumInfo[ 12 ].val.ul  = spec->getScanNumber();				   // Scan Number
		m_spectrumInfo[ 13 ].val.b   = spec->isCentroidMode();				  // Centroided
		m_spectrumInfo[ 14 ].val.f   = spec->getResolution();				   // Resolution
		if( spec->getPolarity() == kome::objects::Spectrum::POLARITY_NEGATIVE ) {
			m_spectrumInfo[ 15 ].val.c = -1;
		}
		else if( spec->getPolarity() == kome::objects::Spectrum::POLARITY_POSITIVE ) {
			m_spectrumInfo[ 15 ].val.c = 1;
		}
		else {
			m_spectrumInfo[ 15 ].val.c = 0;
		}																	   // Polarity
		m_spectrumInfo[ 16 ].val.ms  = st( spec->getTitle() );				  // Title
		m_spectrumInfo[ 17 ].val.b   = spec->hasChromatogram();				 // Chromatogram Flag
		m_spectrumInfo[ 18 ].val.ms  = st( spec->getIcon() );				   // Icon Name

		// write to header
		WRITE_TO_BUFFER( m_spectrumInfo, pos );
		pos += dataSize;
	}

	// write to file
	fwrite( buff, 1, size, m_fp );

	// delete buffer
	delete[] buff;
}

// write spectrum parts
void MsbIO0107::writeSpectrumParts( std::vector< SpectrumPartInfo >& parts ) {
	// spectrum part data size
	unsigned int dataSize = GET_DATA_SIZE( m_partInfo );

	// header
	m_dataHeader[ 0 ].val.ul = GET_DATA_SIZE( m_dataHeader );	// header size
	m_dataHeader[ 1 ].val.ul = dataSize;						 // spectrum part size
	m_dataHeader[ 2 ].val.ul = parts.size();					 // number of spectrum parts

	// write header
	WRITE_TO_FILE( m_dataHeader );
	if( parts.size() == 0 ) {
		return;
	}

	// buffer
	unsigned int size = dataSize * parts.size();
	unsigned char* buff = new unsigned char[ size ];
	unsigned char* pos = buff;

	// write to buffer
	for( unsigned int i = 0; i < parts.size(); i++ ) {
		// spectrum
		SpectrumPartInfo& part = parts[ i ];

		// set data
		m_partInfo[ 0 ].val.ul = part.spectrum;		  // Spectrum ID
		m_partInfo[ 1 ].val.ul = part.length;			// Data Length
		m_partInfo[ 2 ].val.f  = part.start;			 // Start m/z
		m_partInfo[ 3 ].val.f  = part.end;			   // End m/z
		m_partInfo[ 4 ].val.f  = part.totalIntensity;	// Total Intensity
		m_partInfo[ 5 ].val.f  = part.minIntensity;	  // Min Intensity
		m_partInfo[ 6 ].val.f  = part.maxIntensity;	  // Max Intensity
		m_partInfo[ 7 ].val.ul = part.msAlign;		   // m/z Data Align
		m_partInfo[ 8 ].val.ul = part.intAlign;		  // Intensity Data Align

		// write to header
		WRITE_TO_BUFFER( m_partInfo, pos );

		pos += dataSize;
	}

	// write to file
	pos = buff;
	int rest = (int)size;
	while( rest > 0 ) {		
		int writeSize = MIN( rest, 0x10000 );
		int s = fwrite( pos, 1, writeSize, m_fp );
		if( s == 0 ) {
			rest = 0;
		}
		else {
			rest -= s;
			pos += s;
		}
	}

	// delete buffer
	delete[] buff;
}

// write specrum property
void MsbIO0107::writeSpectrumProperties(
		kome::objects::DataSet& dataSet,
		std::map< kome::objects::Spectrum*, unsigned int >& spectrumMap
) {
	// get properties
	std::vector< Prop > properties;

	for( unsigned i = 0; i < dataSet.getNumberOfSpectra(); i++ ) {
		// spectrum
		kome::objects::Spectrum* spec = dataSet.getSpectrum( i );
		unsigned int id = spectrumMap[ spec ];

		// property
		kome::core::Properties& props = spec->getProperties();

		// add to array
		unsigned int idx = properties.size();
		unsigned int num = props.getNumberOfProperties();
		if( num > 0 ) {
			// resize
			properties.resize( idx + num );

			// set value
			for( unsigned int j = 0; j < num; j++ ) {
				Prop& prop = properties[ idx + j ];
				prop.id = id;
				prop.key = st( props.getKey( j ) );
				prop.value = st( props.getValue( j ) );
			}
		}
	}

	// spectrum property data size
	unsigned int dataSize = GET_DATA_SIZE( m_prop );

	// header
	m_dataHeader[ 0 ].val.ul = GET_DATA_SIZE( m_dataHeader );	// header size
	m_dataHeader[ 1 ].val.ul = dataSize;						 // spectrum property size
	m_dataHeader[ 2 ].val.ul = properties.size();				// number of spectrum properties

	// write header
	WRITE_TO_FILE( m_dataHeader );
	if( properties.size() == 0 ) {
		return;
	}

	// buffer
	unsigned int size = dataSize * properties.size();
	unsigned char* buff = new unsigned char[ size ];
	unsigned char* pos = buff;

	// write to buffer
	for( unsigned int i = 0; i < properties.size(); i++ ) {
		// property
		Prop& prop = properties[ i ];

		// set data
		m_prop[ 0 ].val.ul = prop.id;	 // Spectrum ID
		m_prop[ 1 ].val.ms = prop.key;	  // Property Key
		m_prop[ 2 ].val.ms = prop.value;	// Property Value

		// write to header
		WRITE_TO_BUFFER( m_prop, pos );
		pos += dataSize;
	}

	// write to file
	fwrite( buff, 1, size, m_fp );

	// delete buffer
	delete[] buff;
}

// get sample information
void MsbIO0107::onReadSampleInfo( MsbSample& sample ) {
	// read file header
	READ_FROM_FILE( m_fileHeader );

	// create string table
	char* stringTable = NULL;
	createStringTable( &stringTable );

	// get file Header Info
	getFileHeaderInfo( sample, stringTable );

	// delete string table
	if( stringTable != NULL ) {
		delete[] stringTable;
	}
}

// open sample
void MsbIO0107::onReadMsb( MsbSample& sample ) {
	// create string table
	char* stringTable = NULL;
	createStringTable( &stringTable );

	// keep align
	unsigned long long specGroupAlign	 = m_fileHeader[ 6 ].val.ull;
	unsigned long long specGroupPropAlign = m_fileHeader[ 7 ].val.ull;
	unsigned long long spectrumAlign	  = m_fileHeader[ 9 ].val.ull;
	unsigned long long specPartAlign	  = m_fileHeader[ 10 ].val.ull;
	unsigned long long specPropAlign	  = m_fileHeader[ 11 ].val.ull;

	// read spectrum group
	std::map< unsigned int, kome::objects::DataGroupNode* > groupMap;
	fileseek( m_fp, specGroupAlign, SEEK_SET );
	readDataGroupNodes( sample.getRootDataGroupNode(), groupMap, stringTable );

	// read spectrum group properties
	fileseek( m_fp, specGroupPropAlign, SEEK_SET );
	readDataGroupNodeProperties( groupMap, stringTable );

	// read spectrum
	std::map< unsigned int, MsbSpectrum* > specMap;
	fileseek( m_fp, spectrumAlign, SEEK_SET );
	readSpectra( sample, specMap, groupMap, stringTable );
	groupMap.clear();

	// read spectrum part
	fileseek( m_fp, specPartAlign, SEEK_SET );
	readSpectrumParts( specMap );

	// read spectrum property
	fileseek( m_fp, specPropAlign, SEEK_SET );
	readSpectrumProperties( specMap, stringTable );
	specMap.clear();

	// delete string table
	if( stringTable != NULL ) {
		delete[] stringTable;
	}
}

// write msb
bool MsbIO0107::onWriteMsb(
		kome::objects::Sample& sample,
		kome::objects::DataSet& dataSet,
		bool op,
		kome::core::Progress& progress
) {
	// log
	LOG_INFO( FMT( "Writing MSB format sample. [ver 1.7]" ) );

	// get spectra and spectrum groups
	if( progress.isStopped() ) {
		return true;
	}
	std::vector< kome::objects::DataGroupNode* > groups;
	getGroups( dataSet, groups );

	// progress
	unsigned int pos = 0;
	unsigned int length = dataSet.getNumberOfSpectra() + 8;
	progress.setRange( 0, length );
	progress.setPosition( pos );

	// file header
	if( progress.isStopped() ) {
		return true;
	}
	progress.setStatus( "Writing file header" );

	m_fileHeader[ 0 ].val.ul = GET_DATA_SIZE( m_fileHeader );   // Header Size
	m_fileHeader[ 1 ].val.uc = m_bigendian ? 0 : 1;				// Endian
	m_fileHeader[ 4 ].val.ms = st( sample.getSampleSet()->getFilePath() );	 // File Path
	m_fileHeader[ 5 ].val.ms = st( sample.getInstrument() );   // Instrument Name
	m_fileHeader[ 15 ].val.t = time( NULL );					// Time

	WRITE_TO_FILE( m_fileHeader );
	progress.setPosition( ++pos );	// pos == 1

	// spectrum group
	if( progress.isStopped() ) {
		return true;
	}
	progress.setStatus( "Writing spectrum group" );
	m_fileHeader[ 6 ].val.ull = filetell( m_fp );				   // Spectrum Group Align

	std::map< kome::objects::DataGroupNode*, unsigned int > groupMap;
	groupMap[ sample.getRootDataGroupNode() ] = 0;

	writeDataGroupNodes( groups, groupMap );
	progress.setPosition( ++pos );	// pos == 2;

	// spectrum group properties
	if( progress.isStopped() ) {
		return true;
	}
	progress.setStatus( "Writing spectrum group properties" );
	m_fileHeader[ 7 ].val.ull = filetell( m_fp );

//	groups.push_back( sample.getRootDataGroupNode() );
	writeDataGroupNodeProperties( groups, groupMap );
	progress.setPosition( ++pos );  // pos == 3;
	groups.clear();

	// spectrum data
	if( progress.isStopped() ) {
		return true;
	}
	m_fileHeader[ 8 ].val.ull = filetell( m_fp );   // Spectrum Data Align

	kome::core::DataPoints points( kome::core::DataPoints::FLOAT );
	kome::core::DataPoints tmpPts;
	std::map< kome::objects::Spectrum*, unsigned int > spectrumMap;
	std::vector< SpectrumPartInfo > parts;
	for( unsigned int i = 0; i < dataSet.getNumberOfSpectra(); i++ ) {
		// spectrum
		kome::objects::Spectrum* spectrum = dataSet.getSpectrum( i );
		if( progress.isStopped() ) {
			return true;
		}
		progress.setStatus(
			FMT(
				"Writing Spectrum Data [%d/%d] (%s)",
				( i + 1 ),
				dataSet.getNumberOfSpectra(),
				spectrum->getName()
			).c_str()
		);

		// ID
		unsigned int id = ( i + 1 );
		spectrumMap[ spectrum ] = id;

		// gets data points
		spectrum->getXYData( &tmpPts, op );
		for( unsigned int i = 0; i < tmpPts.getLength(); i++ ) {
			// x, y
			double x = tmpPts.getX( i );
			double y = tmpPts.getY( i );

			// add
			if( fabs( y ) > 0.0 ) {
				points.addPoint( x, y );
			}
		}

		// write spectrum data
		writeSpectrumData( id, *spectrum, points, parts );
		progress.setPosition( ++pos );	// pos == 4 + i

		// clear points
		tmpPts.clearPoints();
		points.clearPoints();
	}

	// spectra
	if( progress.isStopped() ) {
		return true;
	}
	m_fileHeader[ 9 ].val.ull = filetell( m_fp );					  // Spectrum Align
	progress.setStatus( "Writing spectrum information." );

	writeSpectra( dataSet, spectrumMap, groupMap );
	progress.setPosition( ++pos );	// pos == 4 + spectra

	// spectrum part
	if( progress.isStopped() ) {
		return true;
	}
	m_fileHeader[ 10 ].val.ull = filetell( m_fp );					  // Spectrum Part Align
	progress.setStatus( "Writing spectrum parts." );

	writeSpectrumParts( parts );
	progress.setPosition( ++pos );	// pos == 5 + spectra
	parts.clear();

	// spectrum property
	if( progress.isStopped() ) {
		return true;
	}
	m_fileHeader[ 11 ].val.ull = filetell( m_fp );					 // Spectrum Property Align
	progress.setStatus( "Writing spectrum properties." );

	writeSpectrumProperties( dataSet, spectrumMap );
	progress.setPosition( ++pos );	// pos == 6 + spectra

	// string table
	if( progress.isStopped() ) {
		return true;
	}
	m_fileHeader[ 12 ].val.ull = filetell( m_fp );					   // String Table Align

	progress.setStatus( "Writing string table." );
	m_fileHeader[ 13 ].val.ul = writeStringTable();				  // Compressed string table seize

	progress.setPosition( ++pos );	// pos == 7 + spectra

	m_fileHeader[ 14 ].val.ul = m_stringTable.size();				// Uncompressed String Table Size
	m_stringTable.clear();

	// rewrite file header
	if( progress.isStopped() ) {
		return true;
	}
	progress.setStatus( "Rewriting file header." );
	fileseek( m_fp, 0, SEEK_SET );
	WRITE_TO_FILE( m_fileHeader );
	progress.setPosition( ++pos );	// pos == 8 + spectra

	return true;
}
