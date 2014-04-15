#include <AbcFrameworkDataTypeConverter.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_iceattributedataarray2D.h>

namespace AbcFramework 
{
	template<>
	class DataTypeConverter<XSI::CString, Alembic::Util::wstring>
	{
	public:
		static void Convert( Alembic::Abc::wstring& out, const XSI::CString& in )
		{
			out = in.GetWideString();
		}
	};

	template<>
	class DataTypeConverter<XSI::MATH::CRotationf, Alembic::Abc::Quatf>
	{
	public:
		__forceinline static void Convert( Alembic::Abc::Quatf& out, const XSI::MATH::CRotationf& in )
		{
			XSI::MATH::CQuaternionf quat = in.GetQuaternion();
			out.r = quat.GetW();
			out.v.x = quat.GetX();
			out.v.y = quat.GetY();
			out.v.z = quat.GetZ();
		}
	};

	template<>
	class DataTypeConverter<XSI::MATH::CRotationf, AbcFramework::VRotationf> {
	public:
		__forceinline static void Convert( AbcFramework::VRotationf& out, const XSI::MATH::CRotationf& in) {
			out.r = (float)in.GetRepresentation();
			switch (in.GetRepresentation())
			{
			case XSI::MATH::CRotationf::siQuaternionRot:
				{
					XSI::MATH::CQuaternionf quat = in.GetQuaternion();
					out.x = quat.GetX();
					out.y = quat.GetY();
					out.z = quat.GetZ();
					out.w = quat.GetW();
				}
				break;
			case XSI::MATH::CRotationf::siEulerRot:
				{
					out.w = (float)in.GetOrder();
					in.GetXYZAngles(out.x, out.y, out.z);
				}
				break;
			case XSI::MATH::CRotationf::siAxisAngleRot:
				{
					XSI::MATH::CVector3f axis = in.GetAxisAngle(out.w);
					out.x = axis.GetX();
					out.y = axis.GetY();
					out.z = axis.GetZ();
				}
				break;
			}
		}
	};

	template<>
	class DataTypeConverter<XSI::MATH::CVector4f, Alembic::Abc::V3f>
	{
	public:
		__forceinline static void Convert( Alembic::Abc::V3f& out, const XSI::MATH::CVector4f& in )
		{
			out.x = in.GetX();
			out.y = in.GetY();
			out.z = in.GetZ();
		}
	};

	template<>
	class DataTypeConverter<XSI::MATH::CRotationf, Alembic::Abc::M33f>
	{
	public:
		__forceinline static void Convert( Alembic::Abc::M33f& out, const XSI::MATH::CRotationf& in )
		{
			XSI::MATH::CMatrix3 mat;

			switch ( in.GetRepresentation() )
			{
			case XSI::MATH::CRotationf::siQuaternionRot:
				{
					XSI::MATH::CQuaternionf quatf = in.GetQuaternion();
					XSI::MATH::CQuaternion quat( quatf.GetW(), quatf.GetX(), quatf.GetY(), quatf.GetZ() );
					mat.SetFromQuaternion( quat );
				}
				break;
			case XSI::MATH::CRotationf::siEulerRot:
				{
					float x, y, z;
					XSI::MATH::CRotationf::RotationOrder order = in.GetOrder();
					in.GetXYZAngles( x, y, z );
					XSI::MATH::CRotation rot;
					rot.SetFromXYZAngles( x, y, z, (XSI::MATH::CRotation::RotationOrder)order );
					mat = rot.GetMatrix();
				}
			case XSI::MATH::CRotationf::siAxisAngleRot:
				{
					float angle;
					XSI::MATH::CVector3f axis3f = in.GetAxisAngle(angle);
					XSI::MATH::CRotation rot;
					rot.SetFromAxisAngle( XSI::MATH::CVector3(axis3f.GetX(),axis3f.GetY(),axis3f.GetZ()), angle );
					mat = rot.GetMatrix();
				}
			}

			out.x[0][0] = (float)mat.GetValue( 0, 0 );
			out.x[0][1] = (float)mat.GetValue( 0, 1 );
			out.x[0][2] = (float)mat.GetValue( 0, 2 );

			out.x[1][0] = (float)mat.GetValue( 1, 0 );
			out.x[1][1] = (float)mat.GetValue( 1, 1 );
			out.x[1][2] = (float)mat.GetValue( 1, 2 );

			out.x[2][0] = (float)mat.GetValue( 2, 0 );
			out.x[2][1] = (float)mat.GetValue( 2, 1 );
			out.x[2][2] = (float)mat.GetValue( 2, 2 );
		}
	};

	template<>
	class DataTypeConverter<XSI::MATH::CMatrix4f, Alembic::Abc::M44f>
	{
	public:
		__forceinline static void Convert( Alembic::Abc::M44f& out, const XSI::MATH::CMatrix4f& in )
		{
			out.x[0][0] = in.GetValue( 0, 0 );
			out.x[0][1] = in.GetValue( 0, 1 );
			out.x[0][2] = in.GetValue( 0, 2 );
			out.x[0][3] = in.GetValue( 0, 3 );

			out.x[1][0] = in.GetValue( 1, 0 );
			out.x[1][1] = in.GetValue( 1, 1 );
			out.x[1][2] = in.GetValue( 1, 2 );
			out.x[1][3] = in.GetValue( 1, 3 );

			out.x[2][0] = in.GetValue( 2, 0 );
			out.x[2][1] = in.GetValue( 2, 1 );
			out.x[2][2] = in.GetValue( 2, 2 );
			out.x[2][3] = in.GetValue( 2, 3 );

			out.x[3][0] = in.GetValue( 3, 0 );
			out.x[3][1] = in.GetValue( 3, 1 );
			out.x[3][2] = in.GetValue( 3, 2 );
			out.x[3][3] = in.GetValue( 3, 3 );
		}
	};

	template<>
	class DataTypeConverter<XSI::MATH::CMatrix4, Alembic::Abc::M44d>
	{
	public:
		__forceinline static void Convert( Alembic::Abc::M44d& out, const XSI::MATH::CMatrix4& in )
		{
			out.x[0][0] = in.GetValue( 0, 0 );
			out.x[0][1] = in.GetValue( 0, 1 );
			out.x[0][2] = in.GetValue( 0, 2 );
			out.x[0][3] = in.GetValue( 0, 3 );

			out.x[1][0] = in.GetValue( 1, 0 );
			out.x[1][1] = in.GetValue( 1, 1 );
			out.x[1][2] = in.GetValue( 1, 2 );
			out.x[1][3] = in.GetValue( 1, 3 );

			out.x[2][0] = in.GetValue( 2, 0 );
			out.x[2][1] = in.GetValue( 2, 1 );
			out.x[2][2] = in.GetValue( 2, 2 );
			out.x[2][3] = in.GetValue( 2, 3 );

			out.x[3][0] = in.GetValue( 3, 0 );
			out.x[3][1] = in.GetValue( 3, 1 );
			out.x[3][2] = in.GetValue( 3, 2 );
			out.x[3][3] = in.GetValue( 3, 3 );
		}
	};

} // namespace AbcFramework


template<	
	class XsiDataT, 
	class AbcTRAITS, 
	class ICEAttributeDataArrayT = XSI::CICEAttributeDataArray<XsiDataT>, 
	class ICEAttributeDataArray2DT = XSI::CICEAttributeDataArray2D<XsiDataT> 
>
class ICEAttributeDataArrayConverter
{
public:
	static void WriteDataArray( XSI::CBaseICEAttributeDataArray* in_pBuffer, IAbcOProperty* in_pOProp )
	{
		CAbcPtr< IAbcOTypedArrayProperty<AbcTRAITS> > spArrayProp = static_cast< IAbcOTypedArrayProperty<AbcTRAITS>* > ( in_pOProp );

		ICEAttributeDataArrayT* pDataForIndexSet = static_cast< ICEAttributeDataArrayT* > ( in_pBuffer );

		// use cast for data types that match Alembic ones

		if ( pDataForIndexSet->GetCount() > 0 )
			spArrayProp->AddSample( (const typename AbcTRAITS::value_type*)&((*pDataForIndexSet)[0]), pDataForIndexSet->GetCount() );
		else
			spArrayProp->AddSample( NULL, 0 );

	}

	static void ConvertAndWriteDataArray2D( XSI::CBaseICEAttributeDataArray* in_pBuffer, IAbcOProperty* in_pOProp )
	{
		// dataArray2D needs to be packed into 1 contiguous block before writing to Alembic

		CAbcPtr< IAbcOTypedArray2DProperty<AbcTRAITS> > spArray2DProp = static_cast< IAbcOTypedArray2DProperty<AbcTRAITS>* > ( in_pOProp );

		CAbcPtr< IAbcOTypedArrayProperty<AbcTRAITS> > spValueProp;
		CAbcPtr< typename IAbcOTypedArray2DProperty<AbcTRAITS>::TheSubArrayIndexPropertyClass > spSubArrayIndicesProp;


		if ( spArray2DProp->GetValueProperty( &spValueProp ) == EResult_Success && spArray2DProp->GetSubArrayIndicesProperty( &spSubArrayIndicesProp ) == EResult_Success )
		{
			ICEAttributeDataArray2DT* p2DDataForIndexSet = static_cast< ICEAttributeDataArray2DT* > ( in_pBuffer );

			// use cast for data types that match Alembic ones
			std::vector<Alembic::Util::uint32_t> subArrayIndices;
			subArrayIndices.reserve( p2DDataForIndexSet->GetCount() * 2 );

			std::vector<AbcTRAITS::value_type> packedArray;
			ULONG subArrayStart = 0;
			for (ULONG subArrayIdx = 0; subArrayIdx < p2DDataForIndexSet->GetCount(); ++subArrayIdx )
			{
				ICEAttributeDataArrayT subArray;
				p2DDataForIndexSet->GetSubArray( subArrayIdx, subArray );
				ULONG subArraySize = subArray.GetCount();

				subArrayIndices.push_back( subArrayStart );
				subArrayIndices.push_back( subArrayStart + subArraySize );
				if ( subArraySize > 0 )
				{
					ConvertDataArray( &subArray, packedArray );
				}
				subArrayStart += subArraySize;
			}

			spValueProp->AddSample( packedArray.data(), (int)packedArray.size() );
			spSubArrayIndicesProp->AddSample( subArrayIndices.data(), (int)subArrayIndices.size() );

		}

	}

	static void ConvertAndWriteDataArray( XSI::CBaseICEAttributeDataArray* in_pBuffer, IAbcOProperty* in_pOProp )
	{
		CAbcPtr< IAbcOTypedArrayProperty<AbcTRAITS> > spArrayProp = static_cast<IAbcOTypedArrayProperty<AbcTRAITS>*>( in_pOProp );

		std::vector<typename AbcTRAITS::value_type> valVec;

		ConvertDataArray( in_pBuffer, valVec );

		spArrayProp->AddSample( valVec.data(), (int)valVec.size() );

	}

	static void ConvertAndWritePerSampleDataArray( XSI::CBaseICEAttributeDataArray* in_pBuffer, IAbcOProperty* in_pOProp, LONG* in_pFaceCounts, ULONG in_NbFaces )
	{
		CAbcPtr< IAbcOTypedArrayProperty<AbcTRAITS> > spArrayProp = static_cast<IAbcOTypedArrayProperty<AbcTRAITS>*>( in_pOProp );

		std::vector<typename AbcTRAITS::value_type> valVec;

		ConvertDataArray( in_pBuffer, valVec );

		if ( valVec.size() > 1 )
			ExportHelper::ReverseFaceWinding<typename AbcTRAITS::value_type>( valVec.data(), (ULONG)valVec.size(), in_pFaceCounts, in_NbFaces );

		spArrayProp->AddSample( valVec.data(), (int)valVec.size() );
	}

	static void ConvertDataArray( XSI::CBaseICEAttributeDataArray* in_pBuffer, std::vector<typename AbcTRAITS::value_type>& io_valVec )
	{
		ICEAttributeDataArrayT* pDataForIndexSet = static_cast< ICEAttributeDataArrayT* > ( in_pBuffer );
		ULONG ulCount = pDataForIndexSet->GetCount();

		if ( ulCount > 0 )
		{
			if ( in_pBuffer->IsConstant() )
			{
				size_t startIdx = io_valVec.size();
				io_valVec.resize( io_valVec.size() + 1 );

				AbcFramework::DataTypeConverter<XsiDataT, typename AbcTRAITS::value_type>::Convert( io_valVec[startIdx], (*pDataForIndexSet)[0] );
			}
			else
			{
				size_t startIdx = io_valVec.size();
				io_valVec.resize( io_valVec.size() + pDataForIndexSet->GetCount() );
				
				for ( ULONG i = 0; i < ulCount; ++i )
				{
					AbcFramework::DataTypeConverter<XsiDataT, typename AbcTRAITS::value_type>::Convert( io_valVec[startIdx + i], (*pDataForIndexSet)[i] );
				}
			}
		}

	}
};

typedef ICEAttributeDataArrayConverter<bool, Alembic::Abc::BooleanTPTraits> ICEAttributeDataArrayBoolConverter;
typedef ICEAttributeDataArrayConverter<LONG, Alembic::Abc::Int32TPTraits> ICEAttributeDataArrayLongConverter;
typedef ICEAttributeDataArrayConverter<float, Alembic::Abc::Float32TPTraits> ICEAttributeDataArrayFloatConverter;

typedef ICEAttributeDataArrayConverter<XSI::MATH::CVector2f, Alembic::Abc::V2fTPTraits> ICEAttributeDataArrayVec2fConverter;
typedef ICEAttributeDataArrayConverter<XSI::MATH::CVector3f, Alembic::Abc::V3fTPTraits> ICEAttributeDataArrayVec3fConverter;

typedef ICEAttributeDataArrayConverter<XSI::MATH::CMatrix3f, Alembic::Abc::M33fTPTraits> ICEAttributeDataArrayMatrix33fConverter;
typedef ICEAttributeDataArrayConverter<XSI::MATH::CMatrix4f, Alembic::Abc::M44fTPTraits> ICEAttributeDataArrayMatrix44fConverter;

typedef ICEAttributeDataArrayConverter<XSI::MATH::CColor4f, Alembic::Abc::C4fTPTraits> ICEAttributeDataArrayColor4fConverter;
typedef ICEAttributeDataArrayConverter<XSI::MATH::CRotationf, Alembic::Abc::QuatfTPTraits> ICEAttributeDataArrayRotationfConverter;
typedef ICEAttributeDataArrayConverter<XSI::MATH::CQuaternionf, Alembic::Abc::QuatfTPTraits> ICEAttributeDataArrayQuatfConverter;

typedef ICEAttributeDataArrayConverter<XSI::MATH::CVector4f,  Alembic::Abc::V3fTPTraits> ICEAttributeDataArrayVec4fConverter;
typedef ICEAttributeDataArrayConverter<XSI::CString, Alembic::Abc::WstringTPTraits, XSI::CICEAttributeDataArrayString, XSI::CICEAttributeDataArray2DString> ICEAttributeDataArrayStringConverter;

static void ConvertAndWriteShapeDataArray( 
	XSI::CBaseICEAttributeDataArray* in_pBuffer, 
	IAbcOProperty* in_pOProp,
	IAbcOPoints* in_pOPoints
	)
{
	EAbcResult abcResult;

	CAbcPtr< IAbcOTypedArrayProperty<Alembic::Abc::Uint8TPTraits> > spShapeTypeProp;
	CAbcPtr< IAbcOTypedArrayProperty<Alembic::Abc::Uint32TPTraits> > spShapeInstanceIdProp;
	CAbcPtr< IAbcOTypedArrayProperty<Alembic::Abc::BooleanTPTraits> > spShapeInstanceHierarchyProp;

	CAbcPtr< IAbcOParticleShapeProperty > spShapeProp = (IAbcOParticleShapeProperty*)in_pOProp;

	abcResult = spShapeProp->GetShapeTypeProperty( (IAbcOProperty**)&spShapeTypeProp );

	abcResult = spShapeProp->GetShapeInstanceIdProperty( (IAbcOProperty**)&spShapeInstanceIdProp );

	abcResult = spShapeProp->GetShapeInstanceHierarchyProperty( (IAbcOProperty**)&spShapeInstanceHierarchyProp );

	if ( spShapeTypeProp != NULL )
	{
		XSI::CICEAttributeDataArray<XSI::MATH::CShape>* pDataForIndexSet = static_cast< XSI::CICEAttributeDataArray<XSI::MATH::CShape>* > ( in_pBuffer );

		ULONG ulSize = pDataForIndexSet->GetCount();

		if ( ulSize > 0 )
		{
			if ( pDataForIndexSet->IsConstant() )
				ulSize = 1;
		}

		std::vector<Alembic::Util::uint8_t> typeVec(ulSize);
		std::vector<Alembic::Util::uint32_t> instanceIdVec;
		std::vector<Alembic::Util::bool_t> instanceHierarchyVec;

		typedef std::map<ULONG, ULONG> ObjectIDMap;
		ObjectIDMap map;


		for ( ULONG i=0; i<ulSize; ++i )
		{
			XSI::MATH::CShape shape = (*pDataForIndexSet)[i];
			XSI::siICEShapeType type = shape.GetType();

			typeVec[i] = type;

			switch ( type )
			{
			case  XSI::siICEShapeReference:
				{
					ULONG ulObjID = shape.GetReferenceDescription().m_nObjectID;
					bool bInstanceHierarchy = shape.GetReferenceDescription().m_bBranch;

					ULONG ulIndex;
					ObjectIDMap::iterator it = map.find(ulObjID);
					if ( it == map.end() )
					{
						map[ulObjID] = ulIndex = in_pOPoints->GetInstancedGeomIndex( ulObjID );
					}
					else
					{
						ulIndex = it->second;
					}

					if ( instanceIdVec.empty() )
					{
						instanceIdVec.resize( ulSize, UINT_MAX );
						instanceHierarchyVec.resize( ulSize, false );
					}

					if ( bInstanceHierarchy )
						in_pOPoints->SetHierarchyInstancedGeom( ulObjID );

					instanceIdVec[i] = ulIndex;
					instanceHierarchyVec[i] = bInstanceHierarchy;
				}
				break;
			case  XSI::siICEShapeInstance:
				{

				}
				break;
			default:
				break;
			}
		}


		spShapeTypeProp->AddSample( typeVec.data(), (int)typeVec.size() );

		if ( spShapeInstanceIdProp )
		{
			if ( !instanceIdVec.empty() )
				spShapeInstanceIdProp->AddSample( instanceIdVec.data(), (int)instanceIdVec.size() );
			else
				spShapeInstanceIdProp->AddSample( NULL, 0 );
		}

		if ( spShapeInstanceHierarchyProp )
		{
			if ( !instanceHierarchyVec.empty() )
				spShapeInstanceHierarchyProp->AddSample( instanceHierarchyVec.data(), (int)instanceHierarchyVec.size() );
			else
				spShapeInstanceHierarchyProp->AddSample( NULL, 0 );
		}

	}

}

void ConvertAndWriteAgeDataArray( 
	XSI::CBaseICEAttributeDataArray* in_pBuffer, 
	IAbcOProperty* in_pOProp,
	XSI::Geometry& in_ptCloudGeom
	)
{

	ULONG ulNbSubSamples = 1;
	
	XSI::ICEAttribute attrNbSubSamples = in_ptCloudGeom.GetICEAttributeFromName( L"NbSubSamples" );
	if( attrNbSubSamples.IsDefined() )
	{
		XSI::CICEAttributeDataArrayLong dataArray;
		attrNbSubSamples.GetDataArray( dataArray );

		if ( dataArray.GetCount() > 0 )
			ulNbSubSamples = dataArray[0];
	}

	XSI::CTime ctime( 1.0 / (double)ulNbSubSamples );
	float fFrameStep = (float)ctime.GetTime( XSI::CTime::Seconds );

	std::vector<Alembic::Abc::float32_t> dataVec;
	ICEAttributeDataArrayFloatConverter::ConvertDataArray( in_pBuffer, dataVec );

	for ( size_t i = 0; i < dataVec.size(); ++i )
	{
		dataVec[i] -= fFrameStep;
	}

	CAbcPtr< IAbcOTypedArrayProperty<Alembic::Abc::Float32TPTraits> > spAgeProp = static_cast< IAbcOTypedArrayProperty<Alembic::Abc::Float32TPTraits>* > ( in_pOProp );
	spAgeProp->AddSample( dataVec.data(), (int)dataVec.size() );
}
