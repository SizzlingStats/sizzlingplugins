
#ifndef STRUCT_PRINTER_H
#define STRUCT_PRINTER_H

namespace StructPrinter
{
	namespace
	{
		void PrintSpace( int numspaces );
		void PrintDataMap( datamap_t *pDatamap, int spacing );

		void PrintSpace( int numspaces )
		{
			for (int i = 0; i < numspaces; ++i)
			{
				Msg(" ");
			}
			Msg("|");
		}

		void PrintDataMap( datamap_t *pDatamap, int spacing )
		{
			while (pDatamap)
			{
				PrintSpace(spacing);
				Msg( "class name: %s\n", pDatamap->dataClassName );
				PrintSpace(spacing);
				Msg( "num fields: %i\n", pDatamap->dataNumFields );

				++spacing;
				int numFields = pDatamap->dataNumFields;
				for (int i = 0; i < numFields; ++i)
				{
					typedescription_t *pTypeDesc = &pDatamap->dataDesc[i];
					PrintTypeDescription(pTypeDesc, spacing+1);
				}
				--spacing;
				pDatamap = pDatamap->baseMap;
			}
		}
	}

	void PrintTypeDescription( typedescription_t *pDesc, int spacing );
	void PrintEntityDatamap( CBaseEntity *pEntity );

	void PrintTypeDescription( typedescription_t *pDesc, int spacing )
	{
		if (pDesc)
		{
			if (pDesc->fieldType == FIELD_VOID)
			{
				PrintSpace(spacing);
				Msg( "field type was void\n" );
			}
			PrintSpace(spacing);
			Msg( "field name   : %s\n", pDesc->fieldName );
			++spacing;
			PrintSpace(spacing);
			Msg( "external name: %s\n", pDesc->externalName );
			PrintSpace(spacing);
			Msg( "offset normal: %i\n", pDesc->fieldOffset[0] );
			PrintSpace(spacing);
			Msg( "offset packed: %i\n", pDesc->fieldOffset[1] );
			PrintDataMap( pDesc->td, spacing+1 );
			--spacing;
		}
	}

	void PrintEntityDatamap( CBaseEntity *pEntity )
	{
		if (pEntity)
		{
			datamap_t *pDatamap = GetDataDescMap( pEntity );
			PrintDataMap( pDatamap, 0 );
		}
	}

}

#endif // STRUCT_PRINTER_H