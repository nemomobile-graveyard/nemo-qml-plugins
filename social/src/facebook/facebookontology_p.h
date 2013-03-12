/*
 * Copyright (C) 2013 Jolla Ltd. <chris.adams@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef FACEBOOKONTOLOGY_P_H
#define FACEBOOKONTOLOGY_P_H

#define FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER QLatin1String("id")
#define FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME       QLatin1String("name")
#define FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE       QLatin1String("object_type")
#define FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTPICTURE    QLatin1String("picture") // XXX TODO.

#define FACEBOOK_ONTOLOGY_LIKE_TARGETIDENTIFIER            QLatin1String("target_id")
#define FACEBOOK_ONTOLOGY_LIKE_USERIDENTIFIER              QLatin1String("id")
#define FACEBOOK_ONTOLOGY_LIKE_USERNAME                    QLatin1String("name")

#define FACEBOOK_ONTOLOGY_PICTURE_SOURCE                   QLatin1String("url")
#define FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE             QLatin1String("is_silhouette")

#define FACEBOOK_ONTOLOGY_TAG_TARGETIDENTIFIER             QLatin1String("target_id")
#define FACEBOOK_ONTOLOGY_TAG_USERIDENTIFIER               QLatin1String("to")
#define FACEBOOK_ONTOLOGY_TAG_TEXT                         QLatin1String("tag_text")
#define FACEBOOK_ONTOLOGY_TAG_XOFFSET                      QLatin1String("x")
#define FACEBOOK_ONTOLOGY_TAG_YOFFSET                      QLatin1String("y")

#define FACEBOOK_ONTOLOGY_METADATA                         QLatin1String("metadata")
#define FACEBOOK_ONTOLOGY_METADATA_CONNECTIONS             QLatin1String("connections")
#define FACEBOOK_ONTOLOGY_METADATA_FIELDS                  QLatin1String("fields")
#define FACEBOOK_ONTOLOGY_METADATA_TYPE                    QLatin1String("type")
#define FACEBOOK_ONTOLOGY_METADATA_ID                      QLatin1String("id")
#define FACEBOOK_ONTOLOGY_METADATA_PAGING                  QLatin1String("paging")
#define FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT             QLatin1String("next")

#define FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES                QLatin1String("likes")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS             QLatin1String("comments")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS               QLatin1String("albums")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS               QLatin1String("photos")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS                 QLatin1String("tags")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS              QLatin1String("friends")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS        QLatin1String("notifications")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_PICTURE              QLatin1String("picture")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_LOCATIONS            QLatin1String("locations")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_EVENTS               QLatin1String("events")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_DATA                 QLatin1String("data")
#define FACEBOOK_ONTOLOGY_CONNECTIONS_SUMMARY              QLatin1String("summary")

#define FACEBOOK_ONTOLOGY_COMMENT                          QLatin1String("comment")
#define FACEBOOK_ONTOLOGY_COMMENT_TYPE                     QLatin1String("type")
#define FACEBOOK_ONTOLOGY_COMMENT_FROM                     QLatin1String("from")
#define FACEBOOK_ONTOLOGY_COMMENT_CREATEDTIME              QLatin1String("created_time")
#define FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT                QLatin1String("like_count")
#define FACEBOOK_ONTOLOGY_COMMENT_LIKED                    QLatin1String("user_likes")
#define FACEBOOK_ONTOLOGY_COMMENT_MESSAGE                  QLatin1String("message")

#define FACEBOOK_ONTOLOGY_PHOTO                            QLatin1String("photo")
#define FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER            QLatin1String("album_id")
#define FACEBOOK_ONTOLOGY_PHOTO_ID                         QLatin1String("id")
#define FACEBOOK_ONTOLOGY_PHOTO_FROM                       QLatin1String("from")
#define FACEBOOK_ONTOLOGY_PHOTO_TAGS                       QLatin1String("tags")
#define FACEBOOK_ONTOLOGY_PHOTO_NAME                       QLatin1String("name")
#define FACEBOOK_ONTOLOGY_PHOTO_NAMETAGS                   QLatin1String("name_tags")
#define FACEBOOK_ONTOLOGY_PHOTO_ICON                       QLatin1String("icon")
#define FACEBOOK_ONTOLOGY_PHOTO_PICTURE                    QLatin1String("picture")
#define FACEBOOK_ONTOLOGY_PHOTO_SOURCE                     QLatin1String("source")
#define FACEBOOK_ONTOLOGY_PHOTO_HEIGHT                     QLatin1String("height")
#define FACEBOOK_ONTOLOGY_PHOTO_WIDTH                      QLatin1String("width")
#define FACEBOOK_ONTOLOGY_PHOTO_IMAGES                     QLatin1String("images")
#define FACEBOOK_ONTOLOGY_PHOTO_LINK                       QLatin1String("link")
#define FACEBOOK_ONTOLOGY_PHOTO_PLACE                      QLatin1String("place")
#define FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME                QLatin1String("created_time")
#define FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME                QLatin1String("updated_time")
#define FACEBOOK_ONTOLOGY_PHOTO_POSITION                   QLatin1String("position")
#define FACEBOOK_ONTOLOGY_PHOTO_LIKED                      QLatin1String("user_likes")

#define FACEBOOK_ONTOLOGY_ALBUM                            QLatin1String("album")
#define FACEBOOK_ONTOLOGY_ALBUM_ID                         QLatin1String("id")
#define FACEBOOK_ONTOLOGY_ALBUM_FROM                       QLatin1String("from")
#define FACEBOOK_ONTOLOGY_ALBUM_NAME                       QLatin1String("name")
#define FACEBOOK_ONTOLOGY_ALBUM_DESCRIPTION                QLatin1String("description")
#define FACEBOOK_ONTOLOGY_ALBUM_LOCATION                   QLatin1String("location")
#define FACEBOOK_ONTOLOGY_ALBUM_LINK                       QLatin1String("link")
#define FACEBOOK_ONTOLOGY_ALBUM_COVERPHOTO                 QLatin1String("cover_photo")
#define FACEBOOK_ONTOLOGY_ALBUM_PRIVACY                    QLatin1String("privacy")
#define FACEBOOK_ONTOLOGY_ALBUM_COUNT                      QLatin1String("count")
#define FACEBOOK_ONTOLOGY_ALBUM_TYPE                       QLatin1String("type")
#define FACEBOOK_ONTOLOGY_ALBUM_CREATEDTIME                QLatin1String("created_time")
#define FACEBOOK_ONTOLOGY_ALBUM_UPDATEDTIME                QLatin1String("updated_time")
#define FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD                  QLatin1String("can_upload")

#define FACEBOOK_ONTOLOGY_USER                             QLatin1String("user")
#define FACEBOOK_ONTOLOGY_USER_ID                          QLatin1String("id")
#define FACEBOOK_ONTOLOGY_USER_NAME                        QLatin1String("name")
#define FACEBOOK_ONTOLOGY_USER_FIRSTNAME                   QLatin1String("first_name")
#define FACEBOOK_ONTOLOGY_USER_MIDDLENAME                  QLatin1String("middle_name")
#define FACEBOOK_ONTOLOGY_USER_LASTNAME                    QLatin1String("last_name")
#define FACEBOOK_ONTOLOGY_USER_GENDER                      QLatin1String("gender")
#define FACEBOOK_ONTOLOGY_USER_LOCALE                      QLatin1String("locale")
#define FACEBOOK_ONTOLOGY_USER_LANGUAGES                   QLatin1String("languages")
#define FACEBOOK_ONTOLOGY_USER_LINK                        QLatin1String("link")
#define FACEBOOK_ONTOLOGY_USER_USERNAME                    QLatin1String("username")
#define FACEBOOK_ONTOLOGY_USER_AGERANGE                    QLatin1String("age_range")
#define FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER        QLatin1String("third_party_id")
#define FACEBOOK_ONTOLOGY_USER_INSTALLED                   QLatin1String("installed")
#define FACEBOOK_ONTOLOGY_USER_TIMEZONEOFFSET              QLatin1String("timezone")
#define FACEBOOK_ONTOLOGY_USER_UPDATEDTIME                 QLatin1String("updated_time")
#define FACEBOOK_ONTOLOGY_USER_VERIFIED                    QLatin1String("verified")
#define FACEBOOK_ONTOLOGY_USER_BIO                         QLatin1String("bio")
#define FACEBOOK_ONTOLOGY_USER_BIRTHDAY                    QLatin1String("birthday")
#define FACEBOOK_ONTOLOGY_USER_COVER                       QLatin1String("cover")
#define FACEBOOK_ONTOLOGY_USER_CURRENCY                    QLatin1String("currency")
#define FACEBOOK_ONTOLOGY_USER_DEVICES                     QLatin1String("devices")
#define FACEBOOK_ONTOLOGY_USER_EDUCATION                   QLatin1String("education")
#define FACEBOOK_ONTOLOGY_USER_EMAIL                       QLatin1String("email")
#define FACEBOOK_ONTOLOGY_USER_HOMETOWN                    QLatin1String("hometown")
#define FACEBOOK_ONTOLOGY_USER_INTERESTEDIN                QLatin1String("interested_in")
#define FACEBOOK_ONTOLOGY_USER_LOCATION                    QLatin1String("location")
#define FACEBOOK_ONTOLOGY_USER_POLITICAL                   QLatin1String("political")
#define FACEBOOK_ONTOLOGY_USER_PAYMENTPRICEPOINTS          QLatin1String("payment_price_points")
#define FACEBOOK_ONTOLOGY_USER_PICTURE                     QLatin1String("picture")
#define FACEBOOK_ONTOLOGY_USER_QUOTES                      QLatin1String("quotes")
#define FACEBOOK_ONTOLOGY_USER_RELATIONSHIPSTATUS          QLatin1String("relationship_status")
#define FACEBOOK_ONTOLOGY_USER_RELIGION                    QLatin1String("religion")
#define FACEBOOK_ONTOLOGY_USER_SECURITYSETTINGS            QLatin1String("security_settings")
#define FACEBOOK_ONTOLOGY_USER_SIGNIFICANTOTHER            QLatin1String("significant_other")
#define FACEBOOK_ONTOLOGY_USER_WEBSITE                     QLatin1String("website")
#define FACEBOOK_ONTOLOGY_USER_WORK                        QLatin1String("work")

#define FACEBOOK_ONTOLOGY_NOTIFICATION                     QLatin1String("notification")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_ID                  QLatin1String("id")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_ID_PREFIX           QLatin1String("notif_")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_FROM                QLatin1String("from")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_TO                  QLatin1String("to")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_APPLICATION         QLatin1String("application")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME         QLatin1String("created_time")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME         QLatin1String("updated_time")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE               QLatin1String("title")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_LINK                QLatin1String("link")
#define FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD              QLatin1String("unread")

#endif // FACEBOOKONTOLOGY_P_H
