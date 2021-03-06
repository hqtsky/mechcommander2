#define OVERLAYBRUSH_CPP
/*************************************************************************************************\
OverlayBrush.cpp			: Implementation of the OverlayBrush component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include "OverlayBrush.h"
#include "action.h"
#include "elist.h"
#include <map>

//-------------------------------------------------------------------------------------------------

class OTMap : public std::map<int32_t, Overlays>
{
};
static OTMap* pOverlaysToTransitionMapping = 0;
class TOMap : public std::map<Overlays, int32_t>
{
};
static TOMap* pTransitionToOverlaysMapping = 0;

static int32_t UP = 0;
static int32_t RIGHT = 1;
static int32_t DOWN = 2;
static int32_t LEFT = 3;

static Overlays
IntToOverlayType(int32_t i)
{
	if (0xff <= i)
	{
		return INVALID_OVERLAY;
	}
	else
	{
		return (Overlays)i;
	}
}

static int32_t
OverlayTypeToInt(Overlays oOverlayType)
{
	if (INVALID_OVERLAY == oOverlayType)
	{
		return 0xff;
	}
	else
	{
		return (int32_t)oOverlayType;
	}
}

static Overlays
overlayType(int32_t mask, int32_t direction)
{
	if ((0 <= direction) && (4 > direction))
	{
		return IntToOverlayType((mask >> (8 * direction)) & 0xff);
	}
	gosASSERT(false);
	return INVALID_OVERLAY;
}

static void
setOverlayType(int32_t& mask, int32_t direction, Overlays overlayType)
{
	if ((0 <= direction) && (4 > direction))
	{
		mask &= ~(0xff << (8 * direction));
		mask |= ((OverlayTypeToInt(overlayType) & 0xff) << (8 * direction));
	}
	else
	{
		gosASSERT(false);
	}
}

#define CONNECTIVITY_MASK(up, right, down, left) \
	((OverlayTypeToInt(up) & 0xff) + ((OverlayTypeToInt(right) & 0xff) << 8) + ((OverlayTypeToInt(down) & 0xff) << 16) + ((OverlayTypeToInt(left) & 0xff) << 24))

static int32_t
connectivityMask(Overlays up, Overlays right, Overlays down, Overlays left)
{
	return CONNECTIVITY_MASK(up, right, down, left);
}

static int32_t
connectivityMask()
{
	return connectivityMask(INVALID_OVERLAY, INVALID_OVERLAY, INVALID_OVERLAY, INVALID_OVERLAY);
}

static int32_t
numberOfOverlayTypes(int32_t mask)
{
	EList<Overlays, Overlays> overlayTypeList;
	int32_t i;
	for (i = 0; i < 4; i++)
	{
		Overlays type = overlayType(mask, i);
		if (INVALID_OVERLAY != type)
		{
			if ((0 == overlayTypeList.Count()) || (EList<Overlays, Overlays>::INVALID_ITERATOR == overlayTypeList.Find(type)))
			{
				overlayTypeList.Append(type);
			}
		}
	}
	return overlayTypeList.Count();
}

/* Non-transition overlays */
static int32_t ConnectivityMaskToIndexOffsetMapping[16] = {
	6, 13, 12, 5, 11, 0, 4, 10, 14, 2, 1, 7, 3, 8, 9, 6};

#define IOM_UP 1
#define IOM_RIGHT 2
#define IOM_DOWN 4
#define IOM_LEFT 8
static int32_t IndexOffsetToConnectivityMaskMapping[15] = {IOM_UP | IOM_DOWN, IOM_RIGHT | IOM_LEFT,
	IOM_UP | IOM_LEFT, IOM_DOWN | IOM_LEFT, IOM_RIGHT | IOM_DOWN, IOM_UP | IOM_RIGHT,
	IOM_UP | IOM_RIGHT | IOM_DOWN | IOM_LEFT, IOM_UP | IOM_RIGHT | IOM_LEFT,
	IOM_UP | IOM_DOWN | IOM_LEFT, IOM_RIGHT | IOM_DOWN | IOM_LEFT, IOM_UP | IOM_RIGHT | IOM_DOWN,
	IOM_DOWN, IOM_RIGHT, IOM_UP, IOM_LEFT};

/* Transition overlays */
/* mh: this array was generated by a GWBASIC program called "trnstns.bas"
   (incase they need to be regenerated) */
static int32_t TransitionConnectivityMaskToIndexOffsetMapping[81] = {-1, -1, -1, -1, -1, 5, -1, 8,
	-1, -1, -1, 2, -1, -1, 30, 6, 18, 22, -1, 0, -1, 9, 32, 13, -1, 24, -1, -1, -1, 11, -1, -1, 17,
	3, 35, 27, -1, -1, 34, -1, -1, 37, 31, 38, 46, 7, 28, 15, 19, 39, 49, 23, 47, 43, -1, 4, -1, 1,
	29, 21, -1, 12, -1, 10, 16, 26, 33, 36, 45, 14, 48, 42, -1, 20, -1, 25, 44, 41, -1, 40, -1};

#define TIOM_UP1 1
#define TIOM_RIGHT1 3
#define TIOM_DOWN1 9
#define TIOM_LEFT1 27
#define TIOM_UP2 2
#define TIOM_RIGHT2 6
#define TIOM_DOWN2 18
#define TIOM_LEFT2 54
static int32_t TransitionIndexOffsetToConnectivityMaskMapping[50] = {TIOM_UP1 + TIOM_DOWN2, // 0
	TIOM_RIGHT1 + TIOM_LEFT2, TIOM_UP2 + TIOM_DOWN1, TIOM_LEFT1 + TIOM_RIGHT2,
	TIOM_UP1 + TIOM_LEFT2, TIOM_UP2 + TIOM_RIGHT1, TIOM_DOWN1 + TIOM_RIGHT2,
	TIOM_LEFT1 + TIOM_DOWN2, TIOM_UP1 + TIOM_RIGHT2, TIOM_RIGHT1 + TIOM_DOWN2,
	TIOM_DOWN1 + TIOM_LEFT2, // 10
	TIOM_LEFT1 + TIOM_UP2, TIOM_UP1 + TIOM_RIGHT2 + TIOM_LEFT2, TIOM_RIGHT1 + TIOM_UP2 + TIOM_DOWN2,
	TIOM_DOWN1 + TIOM_RIGHT2 + TIOM_LEFT2, TIOM_LEFT1 + TIOM_UP2 + TIOM_DOWN2,
	TIOM_UP1 + TIOM_DOWN1 + TIOM_LEFT2, TIOM_RIGHT1 + TIOM_LEFT1 + TIOM_UP2,
	TIOM_UP1 + TIOM_DOWN1 + TIOM_RIGHT2, TIOM_RIGHT1 + TIOM_LEFT1 + TIOM_DOWN2,
	TIOM_UP1 + TIOM_DOWN2 + TIOM_LEFT2, // 20
	TIOM_RIGHT1 + TIOM_UP2 + TIOM_LEFT2, TIOM_DOWN1 + TIOM_UP2 + TIOM_RIGHT2,
	TIOM_LEFT1 + TIOM_RIGHT2 + TIOM_DOWN2, TIOM_UP1 + TIOM_RIGHT2 + TIOM_DOWN2,
	TIOM_RIGHT1 + TIOM_DOWN2 + TIOM_LEFT2, TIOM_DOWN1 + TIOM_UP2 + TIOM_LEFT2,
	TIOM_LEFT1 + TIOM_UP2 + TIOM_RIGHT2, TIOM_UP1 + TIOM_LEFT1 + TIOM_DOWN2,
	TIOM_UP1 + TIOM_RIGHT1 + TIOM_LEFT2,
	TIOM_RIGHT1 + TIOM_DOWN1 + TIOM_UP2, // 30
	TIOM_DOWN1 + TIOM_LEFT1 + TIOM_RIGHT2, TIOM_UP1 + TIOM_RIGHT1 + TIOM_DOWN2,
	TIOM_RIGHT1 + TIOM_DOWN1 + TIOM_LEFT2, TIOM_DOWN1 + TIOM_LEFT1 + TIOM_UP2,
	TIOM_UP1 + TIOM_LEFT1 + TIOM_RIGHT2, TIOM_UP1 + TIOM_RIGHT1 + TIOM_DOWN1 + TIOM_LEFT2,
	TIOM_RIGHT1 + TIOM_DOWN1 + TIOM_LEFT1 + TIOM_UP2,
	TIOM_UP1 + TIOM_DOWN1 + TIOM_LEFT1 + TIOM_RIGHT2,
	TIOM_UP1 + TIOM_RIGHT1 + TIOM_LEFT1 + TIOM_DOWN2,
	TIOM_UP1 + TIOM_RIGHT2 + TIOM_DOWN2 + TIOM_LEFT2, // 40
	TIOM_RIGHT1 + TIOM_UP2 + TIOM_DOWN2 + TIOM_LEFT2,
	TIOM_DOWN1 + TIOM_UP2 + TIOM_RIGHT2 + TIOM_LEFT2,
	TIOM_LEFT1 + TIOM_UP2 + TIOM_RIGHT2 + TIOM_DOWN2,
	TIOM_UP1 + TIOM_RIGHT1 + TIOM_DOWN2 + TIOM_LEFT2,
	TIOM_RIGHT1 + TIOM_DOWN1 + TIOM_UP2 + TIOM_LEFT2,
	TIOM_DOWN1 + TIOM_LEFT1 + TIOM_UP2 + TIOM_RIGHT2,
	TIOM_UP1 + TIOM_LEFT1 + TIOM_RIGHT2 + TIOM_DOWN2,
	TIOM_UP1 + TIOM_DOWN1 + TIOM_RIGHT2 + TIOM_LEFT2,
	TIOM_RIGHT1 + TIOM_LEFT1 + TIOM_UP2 + TIOM_DOWN2};

int32_t
texture(Overlays oOverlayType, int32_t indexOffset)
{
	return ((OverlayTypeToInt(oOverlayType) & 0xffff) << 16) + (indexOffset & 0xffff);
}

Overlays
overlayTypeFromTexture(int32_t iTexture)
{
	return IntToOverlayType((iTexture >> 16) & 0xffff);
}

int32_t
indexOffsetFromTexture(int32_t iTexture)
{
	return (iTexture & 0xffff);
}

static int32_t
ConnectivityMaskToTextureMapping(
	int32_t mask, Overlays currentbrush = INVALID_OVERLAY)
{
	Overlays tmpType = currentbrush;
	if (!((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType)))
	{
		if (INVALID_OVERLAY != currentbrush)
		{
			/* the current brush overlay doesn't support connection, so if the
			mask contains the current brush overlay then just return the (only)
			current brush overlay texture */
			int32_t i;
			for (i = 0; i < 4; i++)
			{
				Overlays oOverlayType = overlayType(mask, i);
				if (currentbrush == oOverlayType)
				{
					int32_t iTexture = texture(currentbrush, 0);
					return iTexture;
				}
			}
		}
	}
	if (RUNWAY == currentbrush)
	{
		/* the runways don't support transitions, so if the mask contains
		runways then remove all other overlay types from the mask */
		int32_t i;
		for (i = 0; i < 4; i++)
		{
			Overlays oOverlayType = overlayType(mask, i);
			if (currentbrush == oOverlayType)
			{
				int32_t i;
				for (i = 0; i < 4; i++)
				{
					Overlays oOverlayType = overlayType(mask, i);
					if (!(RUNWAY == oOverlayType))
					{
						setOverlayType(mask, i, INVALID_OVERLAY);
					}
				}
				break;
			}
		}
	}
	/* here we eliminate overlay types from the mask until there are not more
	 * than two */
	int32_t i = 0;
	while (2 < numberOfOverlayTypes(mask))
	{
		gosASSERT(4 > i);
		if (currentbrush != overlayType(mask, i))
		{
			setOverlayType(mask, i, INVALID_OVERLAY);
		}
		i++;
	}
	int32_t numOverlayTypes = numberOfOverlayTypes(mask);
	gosASSERT(2 >= numOverlayTypes);
	if (2 == numOverlayTypes)
	{
		Overlays overlayType1 = INVALID_OVERLAY;
		Overlays overlayType2 = INVALID_OVERLAY;
		int32_t i = 0;
		while ((INVALID_OVERLAY == overlayType1) || (INVALID_OVERLAY == overlayType2))
		{
			gosASSERT(4 > i);
			if (INVALID_OVERLAY != overlayType(mask, i))
			{
				if (INVALID_OVERLAY == overlayType1)
				{
					overlayType1 = overlayType(mask, i);
				}
				else if (overlayType(mask, i) != overlayType1)
				{
					overlayType2 = overlayType(mask, i);
				}
			}
			i++;
		}
		if (0 == pOverlaysToTransitionMapping)
		{
			pOverlaysToTransitionMapping = new OTMap;
			int32_t pair;
			pair = (OverlayTypeToInt(DIRT_ROAD) << 16) + OverlayTypeToInt(PAVED_ROAD);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_DIRT_PAVED));
			pair = (OverlayTypeToInt(DIRT_ROAD) << 16) + OverlayTypeToInt(TWO_LANE_DIRT_ROAD);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_DIRT_2LANEDIRT));
			pair = (OverlayTypeToInt(DIRT_ROAD) << 16) + OverlayTypeToInt(DAMAGED_ROAD);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_DIRT_DAMAGED));
			pair = (OverlayTypeToInt(TWO_LANE_DIRT_ROAD) << 16) + OverlayTypeToInt(PAVED_ROAD);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_2LANEDIRT_PAVED));
			pair = (OverlayTypeToInt(DAMAGED_ROAD) << 16) + OverlayTypeToInt(PAVED_ROAD);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_DAMAGED_PAVED));
			pair = (OverlayTypeToInt(TWO_LANE_DIRT_ROAD) << 16) + OverlayTypeToInt(DAMAGED_ROAD);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_2LANEDIRT_DAMAGED));
			pair = (OverlayTypeToInt(PAVED_ROAD) << 16) + OverlayTypeToInt(OBRIDGE);
			pOverlaysToTransitionMapping->insert(OTMap::value_type(pair, X_PAVED_ROAD_BRIDGE));
		}
		OTMap::iterator it;
		int32_t pair;
		pair = (OverlayTypeToInt(overlayType1) << 16) + OverlayTypeToInt(overlayType2);
		it = pOverlaysToTransitionMapping->find(pair);
		if (pOverlaysToTransitionMapping->end() == it)
		{
			Overlays swap = overlayType1;
			overlayType1 = overlayType2;
			overlayType2 = swap;
			pair = (OverlayTypeToInt(overlayType1) << 16) + OverlayTypeToInt(overlayType2);
			it = pOverlaysToTransitionMapping->find(pair);
		}
		if (pOverlaysToTransitionMapping->end() == it)
		{
			/* no transition for the pair, eliminate one of the overlay types */
			/* here we eliminate overlay types from the mask until there is not
			 * more than one */
			int32_t i = 0;
			while (1 < numberOfOverlayTypes(mask))
			{
				gosASSERT(4 > i);
				if (currentbrush != overlayType(mask, i))
				{
					setOverlayType(mask, i, INVALID_OVERLAY);
				}
				i++;
			}
			numOverlayTypes = numberOfOverlayTypes(mask);
		}
		else
		{
			Overlays iTextureType = (*it).second;
			int32_t transitionMask = 0;
			int32_t j = 1;
			{
				int32_t i;
				for (i = 0; i < 4; i++)
				{
					Overlays oOverlayType = overlayType(mask, i);
					int32_t a = 0;
					if (overlayType1 == oOverlayType)
					{
						a = 1;
					}
					else if (overlayType2 == oOverlayType)
					{
						a = 2;
					}
					else
					{
						gosASSERT(INVALID_OVERLAY == oOverlayType);
					}
					transitionMask += a * j;
					j *= 3;
				}
			}
			int32_t indexOffset = TransitionConnectivityMaskToIndexOffsetMapping[transitionMask];
			gosASSERT(0 <= indexOffset);
			int32_t iTexture = texture(iTextureType, indexOffset);
			return iTexture;
		}
	}
	if (1 == numOverlayTypes)
	{
		Overlays oOverlayType = INVALID_OVERLAY;
		int32_t i = 0;
		while (true)
		{
			gosASSERT(4 > i);
			if (INVALID_OVERLAY != overlayType(mask, i))
			{
				oOverlayType = overlayType(mask, i);
				break;
			}
			i++;
		}
		int32_t indexOffset = 0;
		Overlays tmpType = oOverlayType;
		if ((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType))
		{
			int32_t nonTransitionMask = 0;
			if (INVALID_OVERLAY != overlayType(mask, UP))
			{
				nonTransitionMask |= IOM_UP;
			}
			if (INVALID_OVERLAY != overlayType(mask, RIGHT))
			{
				nonTransitionMask |= IOM_RIGHT;
			}
			if (INVALID_OVERLAY != overlayType(mask, DOWN))
			{
				nonTransitionMask |= IOM_DOWN;
			}
			if (INVALID_OVERLAY != overlayType(mask, LEFT))
			{
				nonTransitionMask |= IOM_LEFT;
			}
			gosASSERT(0 != nonTransitionMask);
			indexOffset = ConnectivityMaskToIndexOffsetMapping[nonTransitionMask];
		}
		int32_t iTexture = texture(oOverlayType, indexOffset);
		return iTexture;
	}
	else
	{
		gosASSERT(0 == numOverlayTypes);
		return texture(INVALID_OVERLAY, 0);
	}
	gosASSERT(false);
}

static int32_t
TextureToConnectivityMaskMapping(int32_t iTexture)
{
	Overlays oOverlayType = overlayTypeFromTexture(iTexture);
	int32_t indexOffset = indexOffsetFromTexture(iTexture);
	Overlays tmpType = oOverlayType;
	if ((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType))
	{
		gosASSERT(15 > indexOffset);
		int32_t nonTransitionMask = IndexOffsetToConnectivityMaskMapping[indexOffset];
		int32_t mask = connectivityMask();
		int32_t i;
		for (i = 0; i < 4; i++)
		{
			if (0 != (1 & (nonTransitionMask >> i)))
			{
				setOverlayType(mask, i, oOverlayType);
			}
		}
		return mask;
	}
	else if ((X_DIRT_PAVED == tmpType) || (X_DIRT_2LANEDIRT == tmpType) || (X_DIRT_DAMAGED == tmpType) || (X_2LANEDIRT_PAVED == tmpType) || (X_DAMAGED_PAVED == tmpType) || (X_2LANEDIRT_DAMAGED == tmpType) || (X_PAVED_ROAD_BRIDGE == tmpType))
	{
		if (0 == pTransitionToOverlaysMapping)
		{
			pTransitionToOverlaysMapping = new TOMap;
			int32_t pair;
			pair = (OverlayTypeToInt(DIRT_ROAD) << 16) + OverlayTypeToInt(PAVED_ROAD);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_DIRT_PAVED, pair));
			pair = (OverlayTypeToInt(DIRT_ROAD) << 16) + OverlayTypeToInt(TWO_LANE_DIRT_ROAD);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_DIRT_2LANEDIRT, pair));
			pair = (OverlayTypeToInt(DIRT_ROAD) << 16) + OverlayTypeToInt(DAMAGED_ROAD);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_DIRT_DAMAGED, pair));
			pair = (OverlayTypeToInt(TWO_LANE_DIRT_ROAD) << 16) + OverlayTypeToInt(PAVED_ROAD);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_2LANEDIRT_PAVED, pair));
			pair = (OverlayTypeToInt(DAMAGED_ROAD) << 16) + OverlayTypeToInt(PAVED_ROAD);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_DAMAGED_PAVED, pair));
			pair = (OverlayTypeToInt(TWO_LANE_DIRT_ROAD) << 16) + OverlayTypeToInt(DAMAGED_ROAD);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_2LANEDIRT_DAMAGED, pair));
			pair = (OverlayTypeToInt(PAVED_ROAD) << 16) + OverlayTypeToInt(OBRIDGE);
			pTransitionToOverlaysMapping->insert(TOMap::value_type(X_PAVED_ROAD_BRIDGE, pair));
		}
		TOMap::iterator it;
		it = pTransitionToOverlaysMapping->find(oOverlayType);
		if (pTransitionToOverlaysMapping->end() == it)
		{
			gosASSERT(false);
			return connectivityMask();
		}
		int32_t pair = (*it).second;
		Overlays overlayType1 = IntToOverlayType((pair >> 16) & 0xffff);
		Overlays overlayType2 = IntToOverlayType(pair & 0xffff);
		gosASSERT(50 > indexOffset);
		int32_t transitionMask = TransitionIndexOffsetToConnectivityMaskMapping[indexOffset];
		int32_t mask = connectivityMask();
		int32_t i;
		int32_t j = 1;
		for (i = 0; i < 4; i++)
		{
			int32_t a = (transitionMask % (3 * j)) / j;
			if (1 == a)
			{
				setOverlayType(mask, i, overlayType1);
			}
			else if (2 == a)
			{
				setOverlayType(mask, i, overlayType2);
			}
			else
			{
				gosASSERT(0 == a);
			}
			j *= 3;
		}
		return mask;
	}
	else
	{
		int32_t mask = connectivityMask();
		int32_t i;
		for (i = 0; i < 4; i++)
		{
			setOverlayType(mask, i, oOverlayType);
		}
		return mask;
	}
}

static void
boundSafeGetOverlay(int32_t tileR, int32_t tileC, Overlays& type, int32_t& Offset)
{
	if ((0 > tileR) || (0 > tileC) || (Terrain::realVerticesMapSide <= tileR) || (Terrain::realVerticesMapSide <= tileR))
	{
		type = INVALID_OVERLAY;
		Offset = 0;
		return;
	}
	land->getOverlay(tileR, tileC, type, (uint32_t&)Offset);
}

static int32_t
connectivityMaskBasedOnAdjacentTiles(int32_t tileR, int32_t tileC)
{
	Overlays connectsAbove = INVALID_OVERLAY;
	Overlays connectsToTheRight = INVALID_OVERLAY;
	Overlays connectsBelow = INVALID_OVERLAY;
	Overlays connectsToTheLeft = INVALID_OVERLAY;
	Overlays tmpType;
	int32_t tmpOffset;
	boundSafeGetOverlay(tileR - 1, tileC, tmpType, tmpOffset);
	if ((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType) || (X_DIRT_PAVED == tmpType) || (X_DIRT_2LANEDIRT == tmpType) || (X_DIRT_DAMAGED == tmpType) || (X_2LANEDIRT_PAVED == tmpType) || (X_DAMAGED_PAVED == tmpType) || (X_2LANEDIRT_DAMAGED == tmpType) || (X_PAVED_ROAD_BRIDGE == tmpType))
	{
		int32_t iTexture = texture(tmpType, tmpOffset);
		int32_t tmpCM = TextureToConnectivityMaskMapping(iTexture);
		connectsAbove = overlayType(tmpCM, DOWN);
	}
	boundSafeGetOverlay(tileR, tileC + 1, tmpType, tmpOffset);
	if ((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType) || (X_DIRT_PAVED == tmpType) || (X_DIRT_2LANEDIRT == tmpType) || (X_DIRT_DAMAGED == tmpType) || (X_2LANEDIRT_PAVED == tmpType) || (X_DAMAGED_PAVED == tmpType) || (X_2LANEDIRT_DAMAGED == tmpType) || (X_PAVED_ROAD_BRIDGE == tmpType))
	{
		int32_t iTexture = texture(tmpType, tmpOffset);
		int32_t tmpCM = TextureToConnectivityMaskMapping(iTexture);
		connectsToTheRight = overlayType(tmpCM, LEFT);
	}
	boundSafeGetOverlay(tileR + 1, tileC, tmpType, tmpOffset);
	if ((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType) || (X_DIRT_PAVED == tmpType) || (X_DIRT_2LANEDIRT == tmpType) || (X_DIRT_DAMAGED == tmpType) || (X_2LANEDIRT_PAVED == tmpType) || (X_DAMAGED_PAVED == tmpType) || (X_2LANEDIRT_DAMAGED == tmpType) || (X_PAVED_ROAD_BRIDGE == tmpType))
	{
		int32_t iTexture = texture(tmpType, tmpOffset);
		int32_t tmpCM = TextureToConnectivityMaskMapping(iTexture);
		connectsBelow = overlayType(tmpCM, UP);
	}
	boundSafeGetOverlay(tileR, tileC - 1, tmpType, tmpOffset);
	if ((DIRT_ROAD == tmpType) || (PAVED_ROAD == tmpType) || (TWO_LANE_DIRT_ROAD == tmpType) || (DAMAGED_ROAD == tmpType) || (RUNWAY == tmpType) || (OBRIDGE == tmpType) || (DAMAGED_BRIDGE == tmpType) || (X_DIRT_PAVED == tmpType) || (X_DIRT_2LANEDIRT == tmpType) || (X_DIRT_DAMAGED == tmpType) || (X_2LANEDIRT_PAVED == tmpType) || (X_DAMAGED_PAVED == tmpType) || (X_2LANEDIRT_DAMAGED == tmpType) || (X_PAVED_ROAD_BRIDGE == tmpType))
	{
		int32_t iTexture = texture(tmpType, tmpOffset);
		int32_t tmpCM = TextureToConnectivityMaskMapping(iTexture);
		connectsToTheLeft = overlayType(tmpCM, RIGHT);
	}
	int32_t iConnectivityMask =
		connectivityMask(connectsAbove, connectsToTheRight, connectsBelow, connectsToTheLeft);
	return iConnectivityMask;
}

bool
OverlayBrush::paint(Stuff::Vector3D& worldPos, int32_t screenX, int32_t screenY)
{
	int32_t tileC;
	int32_t tileR;
	land->worldToTile(worldPos, tileR, tileC);
	if (tileR < Terrain::realVerticesMapSide && tileR > -1 && tileC < Terrain::realVerticesMapSide && tileC > -1)
	{
		if ((DIRT_ROAD == type) || (PAVED_ROAD == type) || (TWO_LANE_DIRT_ROAD == type) || (DAMAGED_ROAD == type) || (RUNWAY == type) || (OBRIDGE == type) || (DAMAGED_BRIDGE == type) || (X_DIRT_PAVED == type) || (X_DIRT_2LANEDIRT == type) || (X_DIRT_DAMAGED == type) || (X_2LANEDIRT_PAVED == type) || (X_DAMAGED_PAVED == type) || (X_2LANEDIRT_DAMAGED == type) || (X_PAVED_ROAD_BRIDGE == type))
		{
			if (-1 == lastTileR) /* lastTile has not been set yet */
			{
				lastTileR = tileR;
				lastTileC = tileC;
			}
			if ((lastTileR != tileR) || (lastTileC != tileC))
			{
				int32_t brushMotionDirection = -1;
				if (lastTileR == tileR)
				{
					if (lastTileC + 1 == tileC)
					{
						brushMotionDirection = RIGHT;
					}
					else if (lastTileC - 1 == tileC)
					{
						brushMotionDirection = LEFT;
					}
				}
				else if (lastTileC == tileC)
				{
					if (lastTileR + 1 == tileR)
					{
						brushMotionDirection = DOWN;
					}
					else if (lastTileR - 1 == tileR)
					{
						brushMotionDirection = UP;
					}
				}
				if (-1 != brushMotionDirection)
				{
					/* the brush has been dragged to one of the four adjacent
					 * tiles */
					pAction->addChangedVertexInfo(tileR, tileC); // for undo
					pAction->addChangedVertexInfo(lastTileR, lastTileC); // for undo
					Overlays tmpType;
					int32_t tmpOffset;
					/* update last tile */
					int32_t lastTileConnectivityMask = connectivityMask();
					boundSafeGetOverlay(lastTileR, lastTileC, tmpType, tmpOffset);
					int32_t iTexture = texture(tmpType, tmpOffset);
					lastTileConnectivityMask = TextureToConnectivityMaskMapping(iTexture);
					int32_t tmpCM = connectivityMaskBasedOnAdjacentTiles(lastTileR, lastTileC);
					int32_t i;
					for (i = 0; i < 4; i++)
					{
						if (overlayType(lastTileConnectivityMask, i) != overlayType(tmpCM, i))
						{
							setOverlayType(lastTileConnectivityMask, i, INVALID_OVERLAY);
						}
					}
					setOverlayType(lastTileConnectivityMask, brushMotionDirection, type);
					int32_t newLastTileTexture =
						ConnectivityMaskToTextureMapping(lastTileConnectivityMask, type);
					Overlays newLastTileType = overlayTypeFromTexture(newLastTileTexture);
					int32_t newLastTileIndexOffset = indexOffsetFromTexture(newLastTileTexture);
					land->setOverlay(lastTileR, lastTileC, newLastTileType, newLastTileIndexOffset);
					/* update current tile */
					int32_t tileConnectivityMask =
						connectivityMaskBasedOnAdjacentTiles(tileR, tileC);
					int32_t newTileTexture =
						ConnectivityMaskToTextureMapping(tileConnectivityMask, type);
					Overlays newTileType = overlayTypeFromTexture(newTileTexture);
					int32_t newTileIndexOffset = indexOffsetFromTexture(newTileTexture);
					land->setOverlay(tileR, tileC, newTileType, newTileIndexOffset);
				}
			}
		}
		else
		{
			pAction->addChangedVertexInfo(tileR, tileC); // for undo
			land->setOverlay(tileR, tileC, type, offset);
		}
	}
	else
	{
		gosASSERT(false);
	}
	lastTileR = tileR;
	lastTileC = tileC;
	return true;
}

bool
OverlayBrush::beginPaint()
{
	if (pAction)
	{
		gosASSERT(false);
	}
	pAction = new ActionPaintTile;
	gosASSERT(pAction);
	lastTileR = -1;
	lastTileC = -1;
	return true;
}

Action*
OverlayBrush::endPaint()
{
	Action* pRetAction = pAction;
	pAction = nullptr;
	return pRetAction;
}

// end of file ( OverlayBrush.cpp )
